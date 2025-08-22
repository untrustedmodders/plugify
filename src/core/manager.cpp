#include "plugify/core/manager.hpp"
#include "plugify/core/config.hpp"
#include "plugify/core/package.hpp"
#include "plugify/core/plugify.hpp"

#include "core/glaze.hpp"
#include <glaze/glaze.hpp>
#include <utility>

using namespace plugify;

struct Manager::Impl {
	// Plugify
	Plugify& plugify;

	// Configuration
	std::shared_ptr<Config> config;

	// Discovered packages - now stored in sorted order
	std::vector<ModuleInfo> sortedModules;  // Topologically sorted
	std::vector<PluginInfo> sortedPlugins;  // Topologically sorted

	// Original maps for quick lookup
	std::unordered_map<PackageId, ModuleInfo, plg::string_hash, std::equal_to<>> modules;
	std::unordered_map<PackageId, PluginInfo, plg::string_hash, std::equal_to<>> plugins;

	// Loaded instances
	//std::unordered_map<PackageId, std::shared_ptr<Module>, plg::string_hash, std::equal_to<> loadedModules;
	//std::unordered_map<PackageId, std::shared_ptr<Plugin>, plg::string_hash, std::equal_to<> loadedPlugins;
	std::unordered_map<PackageId, bool, plg::string_hash, std::equal_to<>> loadedModules;
	std::unordered_map<PackageId, bool, plg::string_hash, std::equal_to<>> loadedPlugins;

	// Dependency injection components
	//std::unique_ptr<IPackageDiscovery> packageDiscovery;
	//std::unique_ptr<IPackageValidator> packageValidator;
	std::unique_ptr<IDependencyResolver> dependencyResolver;
	//std::unique_ptr<IModuleLoader> moduleLoader;
	//std::unique_ptr<IPluginLoader> pluginLoader;

	// Event handling
	EventDispatcher eventDistpatcher;

	// Dependency tracking (maintained for quick lookup)
	std::unordered_map<PackageId, std::vector<PackageId>, plg::string_hash, std::equal_to<>> pluginToModuleMap;
	std::unordered_map<PackageId, std::vector<PackageId>, plg::string_hash, std::equal_to<>> moduleToPluginsMap;

	// Comprehensive initialization tracking
	InitializationState initState;

	// Retry tracking
	std::unordered_map<PackageId, std::size_t, plg::string_hash, std::equal_to<>> retryCounters;

	// Update performance tracking
    struct UpdateStatistics {
        std::chrono::microseconds lastUpdateTime{0};
        std::chrono::microseconds maxUpdateTime{0};
        std::chrono::microseconds totalUpdateTime{0};
        std::size_t totalUpdates{0};

        double AverageUpdateTime() const {
            return totalUpdates > 0 ?
                static_cast<double>(totalUpdateTime.count()) /  static_cast<double>(totalUpdates) : 0.0;
        }
    } updateStats;

    // Helper methods
    bool ShouldRetry(const EnhancedError& error, std::size_t attemptCount) const {
        if (!error.isRetryable) return false;
        if (config->retryPolicy.retryOnlyTransient && error.category != ErrorCategory::Transient) {
            return false;
        }
        return attemptCount < config->retryPolicy.maxAttempts;
    }

    std::chrono::milliseconds GetRetryDelay(std::size_t attemptCount) const {
        if (!config->retryPolicy.exponentialBackoff) {
            return config->retryPolicy.baseDelay;
        }
		// TODO: clamp
        auto delay = config->retryPolicy.baseDelay * (1 << attemptCount);
        return std::min(delay, config->retryPolicy.maxDelay);
    }

    // Sort packages based on dependency order
    void SortPackagesByDependencies(const DependencyReport& depReport) {
    	// Create ordered lists based on dependency resolution
    	sortedModules.clear();
    	sortedPlugins.clear();

        if (!config->respectDependencyOrder || !depReport.isLoadOrderValid) {
            // Can't sort due to circular dependencies or config
            // Just copy as-is
            for (const auto& [id, info] : modules) {
                if (info->state == PackageState::Validated) {
                    sortedModules.push_back(info);
                }
            }

            for (const auto& [id, info] : plugins) {
                if (info->state == PackageState::Validated) {
                    sortedPlugins.push_back(info);
                }
            }
            return;
        }

        // Use the load order from dependency resolution
        for (const auto& packageId : depReport.loadOrder) {
            // Check if it's a module
            if (auto itModule = modules.find(packageId); itModule != modules.end()) {
                if (itModule->second->state == PackageState::Validated) {
                    sortedModules.push_back(itModule->second);
                }
            }
            // Check if it's a plugin
            else if (auto itPlugin = plugins.find(packageId); itPlugin != plugins.end()) {
                if (itPlugin->second->state == PackageState::Validated) {
                    sortedPlugins.push_back(itPlugin->second);
                }
            }
        }

        std::cout << std::format("Sorted {} modules and {} plugins by dependency order\n",
                                 sortedModules.size(), sortedPlugins.size());
    }
};

Manager::Manager(Plugify& plugify) : _impl(std::make_unique<Impl>(plugify)) {
    // Initialize with default components if not set
    //_impl->packageDiscovery = ManagerFactory::CreateDefaultDiscovery();
    //_impl->packageValidator = ManagerFactory::CreateDefaultValidator();
    _impl->dependencyResolver = ManagerFactory::CreateDefaultResolver();
    //_impl->moduleLoader = ManagerFactory::CreateDefaultModuleLoader();
    //_impl->pluginLoader = ManagerFactory::CreateDefaultPluginLoader();
}

Manager::~Manager() {
	// Destructor no longer performs unloading
	// Users must call terminate() explicitly before destruction
	if (!_impl->loadedModules.empty() || !_impl->loadedPlugins.empty()) {
		std::cerr << "WARNING: Manager destroyed without calling terminate()!\\n";
		std::cerr << "         Call terminate() explicitly for proper shutdown.\\n";
		std::cerr << std::format("         {} modules and {} plugins still loaded!\\n",
								 _impl->loadedModules.size(),
								 _impl->loadedPlugins.size());
	}

	auto _ = Terminate();
}

// ============================================================================
// Dependency Injection
// ============================================================================

/*void Manager::setPackageDiscovery(std::unique_ptr<IPackageValidator> discovery) {
    _impl->packageDiscovery = std::move(discovery);
}

void Manager::setPackageValidator(std::unique_ptr<IPackageValidator> validator) {
    _impl->packageValidator = std::move(validator);
}*/

void Manager::SetDependencyResolver(std::unique_ptr<IDependencyResolver> resolver) {
    _impl->dependencyResolver = std::move(resolver);
}

/*void Manager::setModuleLoader(std::unique_ptr<IModuleLoader> loader) {
    _impl->moduleLoader = std::move(loader);
}

void Manager::setPluginLoader(std::unique_ptr<IPluginLoader> loader) {
    _impl->pluginLoader = std::move(loader);
}*/

// ============================================================================
// Discovery Phase
// ============================================================================

/*template <typename T>
Result<std::shared_ptr<T>> ReadManifest(
		const std::shared_ptr<IFileSystem>& fs,
		const fs::path& path,
		PackageType type
) {
	auto json = fs->ReadTextFile(path);
	if (!json)
		return std::unexpected(json.error());

	auto parsed = glz::read_jsonc<std::shared_ptr<T>>(*json);
	if (!parsed)
		return std::unexpected(glz::format_error(parsed.error(), *json));

	auto& manifest = *parsed;
	manifest->location = path;
	manifest->type = type;
	manifest->id = manifest->name;
	return manifest;
}*/

Result<void> Manager::DiscoverPackages(
    [[maybe_unused]] std::span<const std::filesystem::path> searchPaths
) {
    // Combine provided paths with plugify.configured paths
    std::vector<std::filesystem::path> allPaths;
    allPaths.reserve(searchPaths.size() + _impl->config->searchPaths.size());
    allPaths.insert(allPaths.end(), searchPaths.begin(), searchPaths.end());
    allPaths.insert(allPaths.end(),
                    _impl->config->searchPaths.begin(),
                    _impl->config->searchPaths.end());

    // Discover modules first
	{
		std::string buffer = R"(
[
  {
    "name": "cpp-lang",
    "version": "1.0.0",
    "description": "Core utility functions for other plugins.",
    "author": "qubka",
    "website": "",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [],
    "language": "cpp"
  }
]
)";
		auto s = glz::read_json<std::vector<std::shared_ptr<ModuleManifest>>>(buffer);
		std::vector<ModuleInfo> result;
		for (auto& v : *s) {
			auto info = std::make_shared<Package<ModuleManifest>>();
			info->manifest = v;
			info->manifest->id = info->manifest->name;
			result.push_back(info);
		}

		//auto moduleResult = _impl->packageDiscovery->discoverModules(allPaths);
    	plg::expected<std::vector<ModuleInfo>, std::string> moduleResult = result;
    	if (!moduleResult) {
    		return plg::unexpected(moduleResult.error());
    	}

    	// Store discovered modules
    	for (auto& moduleInfo : moduleResult.value()) {
    		const auto& id = moduleInfo->manifest->id;

    		// Apply whitelist/blacklist filtering
    		if (_impl->config->whitelistedPackages &&
				!std::ranges::contains(*_impl->config->whitelistedPackages, id)) {
    			continue;
			}

    		if (_impl->config->blacklistedPackages &&
				std::ranges::contains(*_impl->config->blacklistedPackages, id)) {
    			UpdatePackageState(id, PackageState::Disabled);
    			continue;
			}

    		EmitEvent({
				.type = EventType::ModuleDiscovered,
				.timestamp = std::chrono::system_clock::now(),
				.packageId = id
			});

    		_impl->modules[id] = std::move(moduleInfo);
    	}
	}
    
    // Discover plugins
	{
		std::string buffer = R"(
[
  {
    "name": "core-utils",
    "version": "1.0.0",
    "description": "Core utility functions for other plugins.",
    "author": "Alice",
    "website": "https://example.com/core-utils",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "graphics-engine",
    "version": "2.1.0",
    "description": "Provides rendering and graphics APIs.",
    "author": "Bob",
    "website": "https://example.com/graphics-engine",
    "license": "Apache-2.0",
    "dependencies": [
      { "name": "core-utils", "constraints": ">=1.0.0" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "physics-engine",
    "version": "3.0.0",
    "description": "Physics simulation engine.",
    "author": "Charlie",
    "website": "https://example.com/physics-engine",
    "license": "GPL-3.0",
    "dependencies": [
      { "name": "core-utils" },
      { "name": "math-lib", "constraints": ">=2.0.0 <3.0.0" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "math-lib",
    "version": "2.5.1",
    "description": "Advanced math functions for simulations.",
    "author": "Diana",
    "website": "https://example.com/math-lib",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [
      { "name": "old-math-lib" }
    ],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "old-math-lib",
    "version": "1.0.0",
    "description": "Deprecated math library.",
    "author": "Eve",
    "website": "https://example.com/old-math-lib",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [
      { "name": "math-lib" }
    ],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "sound-engine",
    "version": "1.2.0",
    "description": "Provides sound APIs.",
    "author": "Frank",
    "website": "https://example.com/sound-engine",
    "license": "MIT",
    "dependencies": [
      { "name": "core-utils" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "ai-module",
    "version": "0.9.0",
    "description": "Artificial intelligence module.",
    "author": "Grace",
    "website": "https://example.com/ai-module",
    "license": "MIT",
    "dependencies": [
      { "name": "math-lib" },
      { "name": "physics-engine", "constraints": ">3.0.0 || <4.0.0" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "network-lib",
    "version": "1.0.0",
    "description": "Networking library.",
    "author": "Henry",
    "website": "https://example.com/network-lib",
    "license": "BSD",
    "dependencies": [],
    "conflicts": [
      { "name": "legacy-network" }
    ],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "legacy-network",
    "version": "0.8.0",
    "description": "Old networking library.",
    "author": "Ian",
    "website": "https://example.com/legacy-network",
    "license": "BSD",
    "dependencies": [],
    "conflicts": [
      { "name": "network-lib" }
    ],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "ui-framework",
    "version": "1.3.2",
    "description": "User interface framework.",
    "author": "Jack",
    "website": "https://example.com/ui-framework",
    "license": "MIT",
    "dependencies": [
      { "name": "graphics-engine" },
      { "name": "network-lib" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "plugin-A",
    "version": "1.0.0",
    "description": "Cyclic test plugin A.",
    "author": "Test",
    "website": "https://example.com/plugin-A",
    "license": "MIT",
    "dependencies": [
      { "name": "plugin-B" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "plugin-B",
    "version": "1.0.0",
    "description": "Cyclic test plugin B.",
    "author": "Test",
    "website": "https://example.com/plugin-B",
    "license": "MIT",
    "dependencies": [
      { "name": "plugin-C" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "plugin-C",
    "version": "1.0.0",
    "description": "Cyclic test plugin C.",
    "author": "Test",
    "website": "https://example.com/plugin-C",
    "license": "MIT",
    "dependencies": [
      { "name": "plugin-A" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "broken-plugin",
    "version": "abc",
    "description": "Plugin with malformed version.",
    "author": "Error",
    "website": "https://example.com/broken-plugin",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "missing-dependency-plugin",
    "version": "1.0.0",
    "description": "Depends on a nonexistent plugin.",
    "author": "Error",
    "website": "https://example.com/missing-dep",
    "license": "MIT",
    "dependencies": [
      { "name": "ghost-plugin" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "deep-A",
    "version": "1.0.0",
    "description": "Deep chain A.",
    "author": "ChainDev",
    "website": "https://example.com/deep-A",
    "license": "MIT",
    "dependencies": [
      { "name": "deep-B" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "deep-B",
    "version": "1.0.0",
    "description": "Deep chain B.",
    "author": "ChainDev",
    "website": "https://example.com/deep-B",
    "license": "MIT",
    "dependencies": [
      { "name": "deep-C" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "deep-C",
    "version": "1.0.0",
    "description": "Deep chain C.",
    "author": "ChainDev",
    "website": "https://example.com/deep-C",
    "license": "MIT",
    "dependencies": [
      { "name": "deep-D" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "deep-D",
    "version": "1.0.0",
    "description": "Deep chain D.",
    "author": "ChainDev",
    "website": "https://example.com/deep-D",
    "license": "MIT",
    "dependencies": [
      { "name": "deep-E" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "deep-E",
    "version": "1.0.0",
    "description": "Deep chain E.",
    "author": "ChainDev",
    "website": "https://example.com/deep-E",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "conflict-A",
    "version": "1.0.0",
    "description": "Conflicts with B.",
    "author": "Tester",
    "website": "https://example.com/conflict-A",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [
      { "name": "conflict-B" }
    ],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "conflict-B",
    "version": "1.0.0",
    "description": "Conflicts with A.",
    "author": "Tester",
    "website": "https://example.com/conflict-B",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [
      { "name": "conflict-A" }
    ],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "multi-version-lib",
    "version": "1.0.0",
    "description": "Library version 1.0.0.",
    "author": "MultiDev",
    "website": "https://example.com/multi-version-lib",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "multi-version-lib",
    "version": "2.0.0",
    "description": "Library version 2.0.0.",
    "author": "MultiDev",
    "website": "https://example.com/multi-version-lib",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "multi-version-consumer",
    "version": "1.0.0",
    "description": "Depends on multi-version-lib >=1.0.0,<2.0.0.",
    "author": "MultiDev",
    "website": "https://example.com/multi-version-consumer",
    "license": "MIT",
    "dependencies": [
      { "name": "multi-version-lib", "constraints": ">=1.0.0 <2.0.0" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "malformed-plugin",
    "version": "1.0.0",
    "author": "MissingFields",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "plugin-X",
    "version": "1.0.0",
    "description": "Part of long cycle.",
    "author": "CycleTester",
    "website": "https://example.com/plugin-X",
    "license": "MIT",
    "dependencies": [
      { "name": "plugin-Y" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "plugin-Y",
    "version": "1.0.0",
    "description": "Part of long cycle.",
    "author": "CycleTester",
    "website": "https://example.com/plugin-Y",
    "license": "MIT",
    "dependencies": [
      { "name": "plugin-Z" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "plugin-Z",
    "version": "1.0.0",
    "description": "Part of long cycle.",
    "author": "CycleTester",
    "website": "https://example.com/plugin-Z",
    "license": "MIT",
    "dependencies": [
      { "name": "plugin-X" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "simple-plugin",
    "version": "0.1.0",
    "description": "Minimal working plugin.",
    "author": "Minimalist",
    "website": "https://example.com/simple-plugin",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "optional-dep-plugin",
    "version": "1.0.0",
    "description": "Has optional dependency.",
    "author": "Tester",
    "website": "https://example.com/optional-dep-plugin",
    "license": "MIT",
    "dependencies": [
      { "name": "math-lib" },
      { "name": "ui-framework", "optional": true }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "redundant-dependency-plugin",
    "version": "1.0.0",
    "description": "Declares dependency twice.",
    "author": "Tester",
    "website": "https://example.com/redundant",
    "license": "MIT",
    "dependencies": [
      { "name": "core-utils" },
      { "name": "core-utils" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "incompatible-engine",
    "version": "1.0.0",
    "description": "Conflicts with graphics-engine <2.0.0.",
    "author": "Tester",
    "website": "https://example.com/incompatible-engine",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [
      { "name": "graphics-engine", "constraints": "<2.0.0" }
    ],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "game-framework",
    "version": "2.0.0",
    "description": "Framework combining many modules.",
    "author": "DevTeam",
    "website": "https://example.com/game-framework",
    "license": "MIT",
    "dependencies": [
      { "name": "graphics-engine" },
      { "name": "physics-engine" },
      { "name": "sound-engine" },
      { "name": "network-lib" },
      { "name": "ui-framework" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "missing-license-plugin",
    "version": "1.0.0",
    "description": "Plugin missing license field.",
    "author": "Oops",
    "website": "https://example.com/missing-license",
    "dependencies": [],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "invalid-deps-plugin",
    "version": "1.0.0",
    "description": "Has malformed dependencies field.",
    "author": "Oops",
    "website": "https://example.com/invalid-deps",
    "license": "MIT",
    "dependencies": [{ "name": "not-a-list" }],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "circular-1",
    "version": "1.0.0",
    "description": "Circular dep test 1.",
    "author": "Test",
    "website": "https://example.com/circular-1",
    "license": "MIT",
    "dependencies": [
      { "name": "circular-2" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "circular-2",
    "version": "1.0.0",
    "description": "Circular dep test 2.",
    "author": "Test",
    "website": "https://example.com/circular-2",
    "license": "MIT",
    "dependencies": [
      { "name": "circular-3" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "circular-3",
    "version": "1.0.0",
    "description": "Circular dep test 3.",
    "author": "Test",
    "website": "https://example.com/circular-3",
    "license": "MIT",
    "dependencies": [
      { "name": "circular-1" }
    ],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "standalone-plugin",
    "version": "1.0.0",
    "description": "No dependencies.",
    "author": "SoloDev",
    "website": "https://example.com/standalone",
    "license": "MIT",
    "dependencies": [],
    "conflicts": [],
    "language": { "name": "cpp" },
    "entry": "test"
  },
  {
    "name": "broken-json-plugin",
    "version": "1.0.0",
    "description": "Plugin with malformed json",
    "author": "Oops",
    "website": "https://example.com/broken-json",
    "license": "MIT",
    "dependencies": [],
    "conflicts": []
  },
  {
    "name": "heavy-plugin",
    "version": "3.2.1",
    "description": "Complex plugin depending on many others.",
    "author": "BigTeam",
    "website": "https://example.com/heavy",
    "license": "MIT",
    "dependencies": [
      { "name": "graphics-engine" },
      { "name": "physics-engine" },
      { "name": "math-lib" },
      { "name": "ai-module" },
      { "name": "network-lib" },
      { "name": "sound-engine" },
      { "name": "ui-framework" }
    ],
    "conflicts": [
      { "name": "legacy-network" },
      { "name": "old-math-lib" }
    ],
    "language": { "name": "cpp" },
    "entry": "test"
  }
]
)";
		auto s = glz::read_json<std::vector<std::shared_ptr<PluginManifest>>>(buffer);
		std::vector<PluginInfo> result;
		if (!s) {
			std::cerr << glz::format_error(s.error(), buffer) << std::endl;
			return std::unexpected(glz::format_error(s.error(), buffer));
		}

		for (auto& v : *s) {
			auto info = std::make_shared<Package<PluginManifest>>();
			info->manifest = v;
			info->manifest->id = info->manifest->name;
			result.push_back(info);
		}

		//auto pluginResult = _impl->packageDiscovery->discoverPlugins(allPaths);
    	plg::expected<std::vector<PluginInfo>, std::string> pluginResult = result;
    	if (!pluginResult) {
    		return plg::unexpected(pluginResult.error());
    	}

    	// Store discovered plugins and map to required language modules
    	for (auto& pluginInfo : pluginResult.value()) {
    		const auto& id = pluginInfo->manifest->id;
            auto language = pluginInfo->manifest->language.GetName();

    		// Apply filtering
    		if (_impl->config->whitelistedPackages &&
				!std::ranges::contains(*_impl->config->whitelistedPackages, id)) {
    			continue;
			}

    		if (_impl->config->blacklistedPackages &&
				std::ranges::contains(*_impl->config->blacklistedPackages, id)) {
    			UpdatePackageState(id, PackageState::Disabled);
    			continue;
			}

    		EmitEvent({
				.type = EventType::PluginDiscovered,
				.timestamp = std::chrono::system_clock::now(),
				.packageId = id
			});

    	    // Map plugin to its required language module
    	    bool moduleFound = false;
    	    for (const auto& [moduleId, moduleInfo] : _impl->modules) {
    	        if (moduleInfo->manifest->language == language) {
    	            _impl->pluginToModuleMap[id].push_back(moduleId);
    	            _impl->moduleToPluginsMap[moduleId].push_back(id);
    	            moduleFound = true;
    	        }
    	    }
        
    	    if (!moduleFound && !_impl->config->loadDisabledPackages) {
    	        pluginInfo->state = PackageState::Error;
    	        pluginInfo->lastError = Error{
    	            .code = ErrorCode::LanguageModuleNotLoaded,
                    .message = std::format("No language module found for language '{}'", language)
                };
    	    }

    		_impl->plugins[id] = std::move(pluginInfo);
    	}
	}

	return {};
}

// ============================================================================
// Initialization Sequence
// ============================================================================

Result<InitializationState> Manager::Initialize() {
    _impl->config = _impl->plugify.GetConfig();
    if (!_impl->config) {
        auto error = EnhancedError::NonRetryable(
            ErrorCode::ConfigurationMissing,
            "Plugify configuration is not set",
            ErrorCategory::Configuration
        );
        return {};//plg::unexpected(error);
    }

	auto _ = DiscoverPackages();

    _impl->initState = {};
    _impl->initState.startTime = std::chrono::system_clock::now();
    auto overallStart = std::chrono::steady_clock::now();
    
    std::cout << "Starting plugin system initialization...\n";
    
    // Step 1: Validate all manifests
    _impl->initState.validationReport = ValidateAllManifests();
    if (!_impl->initState.validationReport.AllPassed() &&
    	!_impl->config->partialStartupMode) {
        auto error = EnhancedError::NonRetryable(
            ErrorCode::ValidationFailed,
            std::format("{} packages failed validation", 
                       _impl->initState.validationReport.FailureCount()),
            ErrorCategory::Validation
        );
        return {};//plg::unexpected(error);
    }
    
    // Step 2: Resolve dependencies and determine load order
    _impl->initState.dependencyReport = ResolveDependencies();
    if (_impl->initState.dependencyReport.HasBlockingIssues() && 
        !_impl->config->partialStartupMode) {
        auto error = EnhancedError::NonRetryable(
            ErrorCode::MissingDependency,
            std::format("Dependency resolution failed: {} blocking issues found",
                       _impl->initState.dependencyReport.BlockerCount()),
            ErrorCategory::Dependency
        );
        return {};//plg::unexpected(error);
    }
    
    // Step 3: Sort packages by dependency order
    _impl->SortPackagesByDependencies(_impl->initState.dependencyReport);
    
    // Step 4: Initialize language modules in dependency order
    auto moduleReport = InitializeModules();
    _impl->initState.initializationReport.moduleInits = moduleReport.moduleInits;
    _impl->initState.initializationReport.totalTime = moduleInitEnd - moduleInitStart;
    
    // Step 5: Initialize plugins in dependency order
    auto pluginReport = InitializePlugins();
    _impl->initState.initializationReport.pluginInits = pluginReport.pluginInits;
    _impl->initState.initializationReport.totalTime = pluginInitEnd - pluginInitStart;

    // Calculate total time
    auto overallEnd = std::chrono::steady_clock::now();
    _impl->initState.totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(overallEnd - overallStart);
    _impl->initState.endTime = std::chrono::system_clock::now();
    
    // Print comprehensive summary (can be disabled via config if needed)
    if (_impl->config->printSummary) {  // Assuming we add this config option
        std::cout << "\n" << _impl->initState.GenerateTextReport() << "\n";
    }
    
    // Determine overall success
    if (_impl->config->partialStartupMode) {
        if (_impl->initState.initializationReport.SuccessCount() > 0) {
            return _impl->initState;  // Partial success
        }
    }
    
    if (_impl->initState.initializationReport.FailureCount() > 0 && !_impl->config->partialStartupMode) {
        auto error = EnhancedError::NonRetryable(
            ErrorCode::InitializationFailed,
            std::format("{} packages failed to initialize", 
                       _impl->initState.initializationReport.FailureCount()),
            ErrorCategory::Runtime
        );
        return {};//plg::unexpected(error);
    }
    
    return _impl->initState;
}

ValidationReport Manager::ValidateAllManifests() {
	ValidationReport report;

	// Validate module manifests
	{
		report.moduleResults.reserve(_impl->modules.size());

		for (auto& [id, moduleInfo] : _impl->modules) {
			if (moduleInfo->state == PackageState::Disabled) continue;

			ValidationReport::PackageValidation validation{.id = id};

			//auto result = true;//_impl->packageValidator->validateManifest(moduleInfo.manifest);
			/*if (!result) {
				validation.passed = false;
				validation.error = EnhancedError::NonRetryable(
					result.error().code,
					result.error().message,
					ErrorCategory::Validation
				);

				moduleInfo->state = PackageState::Error;
				moduleInfo->lastError = result.error();

				std::cerr << std::format("Module '{}' validation failed: {}\n",
										 id, result.error().message);
			} else*/ {
				validation.passed = true;
				moduleInfo->state = PackageState::Validated;
				
				EmitEvent({
					.type = EventType::ModuleValidated,
					.timestamp = std::chrono::system_clock::now(),
					.packageId = id
				});
			}

			report.moduleResults.push_back(std::move(validation));
		}
	}

	// Validate plugin manifests
	{
		report.pluginResults.reserve(_impl->modules.size());

		for (auto& [id, pluginInfo] : _impl->plugins) {
			if (pluginInfo->state == PackageState::Disabled) continue;

			ValidationReport::PackageValidation validation{.id = id};

			//auto result = true;//_impl->packageValidator->validateManifest(pluginInfo.manifest);
			/*if (!result) {
				validation.passed = false;
				validation.error = EnhancedError::NonRetryable(
					result.error().code,
					result.error().message,
					ErrorCategory::Validation
				);

				pluginInfo->state = PackageState::Error;
				pluginInfo->lastError = result.error();

				std::cerr << std::format("Plugin '{}' validation failed: {}\n",
										 id, result.error().message);
			} else*/ {
				validation.passed = true;
				pluginInfo->state = PackageState::Validated;
				
				EmitEvent({
					.type = EventType::PluginValidated,
					.timestamp = std::chrono::system_clock::now(),
					.packageId = id
				});
			}

			report.pluginResults.push_back(std::move(validation));
		}
	}

    return report;
}

DependencyReport Manager::ResolveDependencies() {
    // Collect all validated packages
    PackageCollection allPackages;
    allPackages.reserve(_impl->plugins.size() + _impl->modules.size());

    std::cout << "\n=== Resolve Dependencies ===\n";

    for (auto& [id, info] : _impl->plugins) {
        if (info->state == PackageState::Validated) {
            allPackages[id] = info->manifest;
        }
    }

    for (auto& [id, info] : _impl->modules) {
        if (info->state == PackageState::Validated) {
            allPackages[id] = info->manifest;
        }
    }

    return _impl->dependencyResolver->ResolveDependencies(allPackages);
}

InitializationReport Manager::InitializeModules() {
	InitializationReport report;

    std::cout << "\n=== Initializing Language Modules ===\n";

    // Track successfully loaded modules for dependency checking
    std::unordered_set<PackageId> successfullyLoaded;

    // Use sorted order if available
    //const auto& modulesToInit = _impl->config->respectDependencyOrder && !_impl->sortedModules.empty()
    //                           ? _impl->sortedModules : GetModules(PackageState::Validated);

    for (const auto& moduleInfo : _impl->sortedModules) {
        const auto& id = moduleInfo->manifest->id;

        auto initStart = std::chrono::steady_clock::now();
        InitializationReport::PackageInit init{.id = id};

        // Check if all dependencies are loaded (if configured to do so)
        bool allDependenciesLoaded = true;
        std::vector<PackageId> failedDependencies;

        if (_impl->config->skipDependentsOnFailure && moduleInfo->manifest->dependencies) {
            for (const auto& dependency : *moduleInfo->manifest->dependencies) {
            	const auto& [name, constraints, optional] = *dependency._impl;

                // Skip optional dependencies if configured
                if (optional.value_or(false) && !_impl->config->failOnMissingDependencies) {
                    continue;
                }

                // Check if dependency is loaded
                if (!successfullyLoaded.contains(name) &&
                    !_impl->loadedPlugins.contains(name)) {

                    allDependenciesLoaded = false;
                    failedDependencies.push_back(name);
                }
            }
        }

        if (!allDependenciesLoaded) {
            // Mark as skipped due to failed dependencies
            moduleInfo->state = PackageState::Skipped;
            auto error = EnhancedError::NonRetryable(
                ErrorCode::MissingDependency,
                std::format("Cannot load module '{}' because dependencies failed to load: [{}]",
                           id, plg::join(failedDependencies, ", ")),
                ErrorCategory::Dependency
            );
            moduleInfo->lastError = error;

            init.finalState = PackageState::Skipped;
            init.error = error;
            init.loadTime = std::chrono::milliseconds{0};
            report.moduleInits.push_back(std::move(init));

            EmitEvent({
                .type = EventType::ModuleFailed,
                .timestamp = std::chrono::system_clock::now(),
                .packageId = id,
                .error = error
            });

            std::cerr << std::format("  ⚠️  Module '{}' skipped: dependencies not available [{}]\n",
                                     id, plg::join(failedDependencies, ", "));

            // Continue to next module unless strict mode
            if (!_impl->config->partialStartupMode) {
                break;
            }
            continue;
        }

        moduleInfo->state = PackageState::Initializing;

        EmitEvent({
            .type = EventType::ModuleLoading,
            .timestamp = std::chrono::system_clock::now(),
            .packageId = id
        });

        // Attempt to load with intelligent retry
        bool loadSuccess = false;
        std::size_t attemptCount = 0;
        EnhancedError lastError;

        while (!loadSuccess) {
            attemptCount++;
            init.retryAttempts = attemptCount - 1;

            if (attemptCount > 1) {
                auto delay = _impl->GetRetryDelay(attemptCount - 2);
                std::cout << std::format("  Retry {}/{} for module '{}' (waiting {}ms)\n",
                                        attemptCount - 1,
                                        _impl->config->retryPolicy.maxAttempts,
                                        id, delay.count());
                std::this_thread::sleep_for(delay);
            }

            // Load the module
            //auto moduleResult = true;//_impl->moduleLoader->loadModule(moduleInfo->manifest);
            /*if (!moduleResult) {
                lastError = EnhancedError::Transient(
                    moduleResult.error().code,
                    moduleResult.error().message
                );

                if (!_impl->ShouldRetry(lastError, attemptCount)) {
                    break;
                }
                continue;
            }*/

            // Initialize the module
            //auto initResult = true;/*_impl->moduleLoader->initializeModule(
               // *moduleResult.value(), moduleInfo->manifest);*/

            /*if (!initResult) {
                if (initResult.error().code == ErrorCode::InitializationFailed) {
                    lastError = EnhancedError::Transient(
                        initResult.error().code,
                        initResult.error().message
                    );
                } else {
                    lastError = EnhancedError::NonRetryable(
                        initResult.error().code,
                        initResult.error().message,
                        ErrorCategory::Configuration
                    );
                }

                if (!_impl->ShouldRetry(lastError, attemptCount)) {
                    break;
                }
                continue;
            }*/

            // Success!
            _impl->loadedModules[id] = true;//std::move(moduleResult.value());
            moduleInfo->state = PackageState::Ready;
            loadSuccess = true;
            init.finalState = PackageState::Ready;
            successfullyLoaded.insert(id);  // Mark as successfully loaded

            EmitEvent({
                .type = EventType::ModuleLoaded,
                .timestamp = std::chrono::system_clock::now(),
                .packageId = id
            });

            std::cout << std::format("  ✓ Module '{}' loaded successfully", id);
            if (attemptCount > 1) {
                std::cout << std::format(" (after {} retries)", attemptCount - 1);
            }

            if (moduleInfo->manifest->dependencies && !moduleInfo->manifest->dependencies->empty()) {
                /*std::cout << std::format(" [deps: {}]",
            		plg::join(*moduleInfo->manifest->dependencies, &Dependency::GetName, ", "));*/
            }
            std::cout << "\n";
        }

        if (!loadSuccess) {
            moduleInfo->state = PackageState::Error;
            moduleInfo->lastError = lastError;
            init.finalState = PackageState::Error;
            init.error = lastError;

            EmitEvent({
                .type = EventType::ModuleFailed,
                .timestamp = std::chrono::system_clock::now(),
                .packageId = id,
                .error = lastError
            });

            std::cerr << std::format("  ✗ Failed to load module '{}': {} ({})\n",
                                     id, lastError.message,
                                     lastError.isRetryable ? "gave up after retries" : "non-retryable");

            // Check if any packages depend on this failed module
            if (_impl->initState.dependencyReport.reverseDependencyGraph.contains(id)) {
                const auto& dependents = _impl->initState.dependencyReport.reverseDependencyGraph.at(id);
                if (!dependents.empty()) {
                    std::cerr << std::format("     ⚠️  This will prevent loading: [{}]\n",
                        plg::join(dependents, ", "));
                }
            }
        }

        auto initEnd = std::chrono::steady_clock::now();
        init.loadTime = std::chrono::duration_cast<std::chrono::milliseconds>(initEnd - initStart);
        report.moduleInits.push_back(std::move(init));
    }

    // Final pass: Mark any modules that were never attempted as skipped
    for (auto& [id, moduleInfo] : _impl->modules) {
        if (moduleInfo->state == PackageState::Validated) {
            // This module was never attempted
            moduleInfo->state = PackageState::Skipped;
            moduleInfo->lastError = Error{
                .code = ErrorCode::MissingDependency,
                .message = "Skipped due to initialization order or dependency cascade"
            };

            // Add to report
            InitializationReport::PackageInit init{
                .id = id,
                .finalState = PackageState::Skipped,
                .retryAttempts = 0,
                .error = EnhancedError::NonRetryable(
                    ErrorCode::MissingDependency,
                    "Never attempted due to dependency cascade",
                    ErrorCategory::Dependency
                ),
                .loadTime = std::chrono::milliseconds{0}
            };
            report.moduleInits.push_back(std::move(init));
        }
    }

    return report;
}

InitializationReport Manager::InitializePlugins() {
	InitializationReport report;

    std::cout << "\n=== Initializing Plugins ===\n";

    // Track successfully loaded plugins for dependency checking
    std::unordered_set<PackageId> successfullyLoaded;

    // Add all successfully loaded modules to the set
    for (const auto& [id, _] : _impl->loadedModules) {
        successfullyLoaded.insert(id);
    }

    // Use sorted order if available
    //const auto& pluginsToInit = _impl->config->respectDependencyOrder && !_impl->sortedPlugins.empty()
    //                           ? _impl->sortedPlugins
    //                           : GetPlugins(PackageState::Validated);

    for (const auto& pluginInfo : _impl->sortedPlugins) {
        const auto& id = pluginInfo->manifest->id;

        auto initStart = std::chrono::steady_clock::now();
        InitializationReport::PackageInit init{.id = id};

        // First check if all dependencies are loaded (if configured to do so)
        bool allDependenciesLoaded = true;
        std::vector<PackageId> failedDependencies;

        if (pluginInfo->manifest->dependencies && _impl->config->skipDependentsOnFailure) {
            for (const auto& dependency : *pluginInfo->manifest->dependencies) {
            	const auto& [name, constraints, optional] = *dependency._impl;

                // Skip optional dependencies if configured
                if (optional.value_or(false) && !_impl->config->failOnMissingDependencies) {
                    continue;
                }

                // Check if dependency is loaded
                if (!successfullyLoaded.contains(name) &&
                    !_impl->loadedPlugins.contains(name)) {

                    allDependenciesLoaded = false;
                    failedDependencies.push_back(name);
                }
            }
        }

        if (!allDependenciesLoaded) {
            // Mark as skipped due to failed dependencies
            pluginInfo->state = PackageState::Skipped;
            auto error = EnhancedError::NonRetryable(
                ErrorCode::MissingDependency,
                std::format("Cannot load plugin '{}' because dependencies failed to load: [{}]",
                           id, plg::join(failedDependencies, ", ")),
                ErrorCategory::Dependency
            );
            pluginInfo->lastError = error;

            init.finalState = PackageState::Skipped;
            init.error = error;
            init.loadTime = std::chrono::milliseconds{0};
            report.pluginInits.push_back(std::move(init));

            EmitEvent({
                .type = EventType::PluginFailed,
                .timestamp = std::chrono::system_clock::now(),
                .packageId = id,
                .error = error
            });

            std::cerr << std::format("  ⚠️  Plugin '{}' skipped: dependencies not available [{}]\n",
                                     id, plg::join(failedDependencies, ", "));

            // Continue to next plugin unless strict mode
            if (!_impl->config->partialStartupMode) {
                break;
            }
            continue;
        }

        // Check if required language module is loaded
        auto moduleIds = _impl->pluginToModuleMap.find(id);
        if (moduleIds == _impl->pluginToModuleMap.end() || moduleIds->second.empty()) {
            pluginInfo->state = PackageState::Skipped;
            auto error = EnhancedError::NonRetryable(
                ErrorCode::LanguageModuleNotLoaded,
                std::format("No language module available for plugin '{}'", id),
                ErrorCategory::Dependency
            );
            pluginInfo->lastError = error;

            init.finalState = PackageState::Skipped;
            init.error = error;
            init.loadTime = std::chrono::milliseconds{0};
            report.pluginInits.push_back(std::move(init));

            std::cerr << std::format("  ✗ Plugin '{}' skipped: no language module\n", id);
            continue;
        }

        // Find the loaded module for this plugin's language
       // Module* languageModule = nullptr;
		bool languageModule = false;
        std::string usedModuleId;

        for (const auto& moduleId : moduleIds->second) {
            auto it = _impl->loadedModules.find(moduleId);
            if (it != _impl->loadedModules.end()) {
                languageModule = it->second;//.get();
                usedModuleId = moduleId;
                break;
            }
        }

        if (!languageModule) {
            pluginInfo->state = PackageState::Skipped;
            auto error = EnhancedError::NonRetryable(
                ErrorCode::LanguageModuleNotLoaded,
                std::format("Language module not loaded for plugin '{}' (required module failed to initialize)", id),
                ErrorCategory::Dependency
            );
            pluginInfo->lastError = error;

            init.finalState = PackageState::Skipped;
            init.error = error;
            init.loadTime = std::chrono::milliseconds{0};
            report.pluginInits.push_back(std::move(init));

            std::cerr << std::format("  ⚠️  Plugin '{}' skipped: language module failed to load\n", id);

            // Show which language module was needed
            if (!moduleIds->second.empty()) {
                std::cerr << std::format("     Required module: '{}'\n", moduleIds->second[0]);
            }

            continue;
        }

        pluginInfo->state = PackageState::Initializing;

        EmitEvent({
            .type = EventType::PluginLoading,
            .timestamp = std::chrono::system_clock::now(),
            .packageId = id
        });

        // Attempt to load with intelligent retry
        bool loadSuccess = false;
        std::size_t attemptCount = 0;
        EnhancedError lastError;

        while (!loadSuccess) {
            attemptCount++;
            init.retryAttempts = attemptCount - 1;

            if (attemptCount > 1) {
                auto delay = _impl->GetRetryDelay(attemptCount - 2);
                std::cout << std::format("  Retry {}/{} for plugin '{}' (waiting {}ms)\n",
                                        attemptCount - 1,
                                        _impl->config->retryPolicy.maxAttempts,
                                        id, delay.count());
                std::this_thread::sleep_for(delay);
            }

            // Load the plugin through its language module
            //auto pluginResult = true;/*_impl->pluginLoader->loadPlugin(
                //pluginInfo->manifest, *languageModule);*/

            /*if (!pluginResult) {
                lastError = EnhancedError::Transient(
                    pluginResult.error().code,
                    pluginResult.error().message
                );

                if (!_impl->ShouldRetry(lastError, attemptCount)) {
                    break;
                }
                continue;
            }*/

            // Initialize the plugin
           // auto initResult = true;/*_impl->pluginLoader->initializePlugin(
                //*pluginResult.value(), pluginInfo->manifest);*/

            /*if (!initResult) {
                if (initResult.error().code == ErrorCode::InitializationFailed) {
                    lastError = EnhancedError::Transient(
                        initResult.error().code,
                        initResult.error().message
                    );
                } else {
                    lastError = EnhancedError::NonRetryable(
                        initResult.error().code,
                        initResult.error().message,
                        ErrorCategory::Configuration
                    );
                }

                if (!_impl->ShouldRetry(lastError, attemptCount)) {
                    break;
                }
                continue;
            }*/

            // Success!
            _impl->loadedPlugins[id] = true;//std::move(pluginResult.value());
            pluginInfo->state = PackageState::Started;
            loadSuccess = true;
            init.finalState = PackageState::Started;
            successfullyLoaded.insert(id);  // Mark as successfully loaded

            EmitEvent({
                .type = EventType::PluginLoaded,
                .timestamp = std::chrono::system_clock::now(),
                .packageId = id
            });

			// TODO

            EmitEvent({
                .type = EventType::PluginStarted,
                .timestamp = std::chrono::system_clock::now(),
                .packageId = id
            });

            std::cout << std::format("  ✓ Plugin '{}' loaded successfully (module: '{}')",
                                     id, usedModuleId);
            if (attemptCount > 1) {
                std::cout << std::format(" [{}  retries]", attemptCount - 1);
            }
            if (pluginInfo->manifest->dependencies && !pluginInfo->manifest->dependencies->empty()) {
                /*std::cout << std::format(" [deps: {}]",
                    plg::join(*pluginInfo->manifest->dependencies, &Dependency::GetName, ", "));*/
            }
            std::cout << "\n";
        }

        if (!loadSuccess) {
            pluginInfo->state = PackageState::Error;
            pluginInfo->lastError = lastError;
            init.finalState = PackageState::Error;
            init.error = lastError;

            EmitEvent({
                .type = EventType::PluginFailed,
                .timestamp = std::chrono::system_clock::now(),
                .packageId = id,
                .error = lastError
            });

            std::cerr << std::format("  ✗ Failed to load plugin '{}': {} ({})\n",
                                     id, lastError.message,
                                     lastError.isRetryable ? "gave up after retries" : "non-retryable");

            // Check if any packages depend on this failed plugin
            if (_impl->initState.dependencyReport.reverseDependencyGraph.contains(id)) {
                const auto& dependents = _impl->initState.dependencyReport.reverseDependencyGraph.at(id);
                if (!dependents.empty()) {
                    std::cerr << std::format("     ⚠️  This will prevent loading: [{}]\n",
                        plg::join(dependents, ", "));
                }
            }
        }

        auto initEnd = std::chrono::steady_clock::now();
        init.loadTime = std::chrono::duration_cast<std::chrono::milliseconds>(initEnd - initStart);
        report.pluginInits.push_back(std::move(init));
    }

    // Final pass: Mark any packages that were never attempted as skipped
    std::size_t cascadeSkipped = 0;
    for (auto& [id, pluginInfo] : _impl->plugins) {
        if (pluginInfo->state == PackageState::Validated) {
            // This plugin was never attempted due to cascade failure
            pluginInfo->state = PackageState::Skipped;
            pluginInfo->lastError = Error{
                .code = ErrorCode::MissingDependency,
                .message = "Skipped due to initialization order or dependency cascade"
            };

            // Add to report if not already there
            bool alreadyInReport = std::ranges::any_of(report.pluginInits,
                [&id](const auto& init) { return init.id == id; });

            if (!alreadyInReport) {
                InitializationReport::PackageInit init{
                    .id = id,
                    .finalState = PackageState::Skipped,
                    .retryAttempts = 0,
                    .error = EnhancedError::NonRetryable(
                        ErrorCode::MissingDependency,
                        "Never attempted due to dependency cascade",
                        ErrorCategory::Dependency
                    ),
                    .loadTime = std::chrono::milliseconds{0}
                };
                report.pluginInits.push_back(std::move(init));
                cascadeSkipped++;
            }
        }
    }

    if (cascadeSkipped > 0) {
        std::cout << std::format("\n  ℹ️  {} additional plugins were skipped due to dependency cascade failures\n",
                                 cascadeSkipped);
    }
    
    return report;
}

// ============================================================================
// Termination Sequence
// ============================================================================
Result<void> Manager::Terminate() {
	// Unload in reverse dependency order for clean shutdown
	// Plugins depend on modules, so unload plugins first
    
	// If we have a valid load order, unload in reverse
	if (_impl->initState.dependencyReport.isLoadOrderValid) {
		const auto& loadOrder = _impl->initState.dependencyReport.loadOrder;
        
		// Reverse order for unloading
		for (auto it = loadOrder.rbegin(); it != loadOrder.rend(); ++it) {
			const auto& id = *it;
            
			// Unload plugin if loaded
			if (auto pluginIt = _impl->loadedPlugins.find(id); 
				pluginIt != _impl->loadedPlugins.end()) {
				EmitEvent({
					.type = EventType::PluginEnded,
					.timestamp = std::chrono::system_clock::now(),
					.packageId = id
				});
				//_impl->pluginLoader->unloadPlugin(std::move(pluginIt->second));
				_impl->loadedPlugins.erase(pluginIt);
			}
            
			// Unload module if loaded
			if (auto moduleIt = _impl->loadedModules.find(id); 
				moduleIt != _impl->loadedModules.end()) {
				//_impl->moduleLoader->unloadModule(std::move(moduleIt->second));
				_impl->loadedModules.erase(moduleIt);
			}
		}
	} else {
		// Fallback: unload all plugins first, then modules
		for (auto& [id, plugin] : _impl->loadedPlugins) {
			EmitEvent({
				.type = EventType::PluginEnded,
				.timestamp = std::chrono::system_clock::now(),
				.packageId = id
			});
			//_impl->pluginLoader->unloadPlugin(std::move(plugin));
		}
		_impl->loadedPlugins.clear();
        
		for (auto& [id, module] : _impl->loadedModules) {
			//_impl->moduleLoader->unloadModule(std::move(module));
		}
		_impl->loadedModules.clear();
	}

	return {};
}

// ============================================================================
// Query Methods
// ============================================================================

ModuleInfo Manager::GetModule(std::string_view moduleId) const {
    auto it = _impl->modules.find(moduleId);
    if (it != _impl->modules.end()) {
        return it->second;
    }
    return {};
}

PluginInfo Manager::GetPlugin(std::string_view pluginId) const {
    auto it = _impl->plugins.find(pluginId);
    if (it != _impl->plugins.end()) {
        return it->second;
    }
    return {};
}

std::vector<ModuleInfo> Manager::GetModules() const {
    std::vector<ModuleInfo> result;
    result.reserve(_impl->modules.size());
    
    for (const auto& [id, info] : _impl->modules) {
    	result.push_back(info);
    }
    
    return result;
}

std::vector<PluginInfo> Manager::GetPlugins() const {
    std::vector<PluginInfo> result;
    result.reserve(_impl->plugins.size());
    
    for (const auto& [id, info] : _impl->plugins) {
    	result.push_back(info);
    }
    
    return result;
}

std::vector<ModuleInfo> Manager::GetModules(PackageState state) const {
    std::vector<ModuleInfo> result;
    result.reserve(_impl->modules.size());
    
    for (const auto& [id, info] : _impl->modules) {
    	if (info->state == state) {
    		result.push_back(info);
    	}
    }
    
    return result;
}

std::vector<PluginInfo> Manager::GetPlugins(PackageState state) const {
    std::vector<PluginInfo> result;
    result.reserve(_impl->plugins.size());
    
    for (const auto& [id, info] : _impl->plugins) {
    	if (info->state == state) {
    		result.push_back(info);
    	}
    }
    
    return result;
}

bool Manager::IsModuleLoaded(std::string_view moduleId) const {
    return _impl->loadedModules.contains(moduleId);
}

bool Manager::IsPluginLoaded(std::string_view pluginId) const {
    return _impl->loadedPlugins.contains(pluginId);
}

// ============================================================================
// Event Handling
// ============================================================================

UniqueId Manager::Subscribe(EventHandler handler) {
	return _impl->eventDistpatcher.Subscribe(std::move(handler));
}

void Manager::Unsubscribe(UniqueId id) {
    _impl->eventDistpatcher.Unsubscribe(id);
}

void Manager::EmitEvent(const Event& event) {
    _impl->eventDistpatcher.Emit(event);
}

// ============================================================================
// Helper Methods
// ============================================================================

void Manager::UpdatePackageState(const PackageId& id, PackageState newState) {
    if (auto itModule = _impl->modules.find(id); itModule != _impl->modules.end()) {
        itModule->second->state = newState;
    } else if (auto itPlugin = _impl->plugins.find(id); itPlugin != _impl->plugins.end()) {
        itPlugin->second->state = newState;
    }
}

std::vector<PackageId> Manager::GetPluginsForModule(std::string_view moduleId) const {
    auto it = _impl->moduleToPluginsMap.find(moduleId);
    if (it != _impl->moduleToPluginsMap.end()) {
        return it->second;
    }
    return {};
}

std::unique_ptr<IDependencyResolver> ManagerFactory::CreateDefaultResolver() {
    return std::make_unique<LibsolvDependencyResolver>();
}