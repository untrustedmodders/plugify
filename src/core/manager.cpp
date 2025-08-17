#include "manager.hpp"
#include "json.hpp"
#include "module.hpp"
#include "module_manifest.hpp"
#include "plugify.hpp"
#include "plugify/api/plugin_manifest.hpp"
#include "plugin.hpp"
#include "plugin_manifest.hpp"

#include <plugify/api/dependency.hpp>
#include <plugify/api/manager.hpp>
#include <plugify/api/plugify.hpp>

using namespace plugify;

Manager::Manager(Plugify& plugify) : Context(plugify) {
}

Manager::~Manager() {
	Terminate();
}

bool Manager::Initialize() {
	if (IsInitialized())
		return false;

	auto debugStart = DateTime::Now();

	DiscoverAllModulesAndPlugins();
	LoadRequiredLanguageModules();
	LoadAndStartAvailablePlugins();

	_inited = true;

	PL_LOG_DEBUG("PluginManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
	return true;
}

void Manager::Terminate() {
	if (!IsInitialized())
		return;

	TerminateAllPlugins();
	TerminateAllModules();

	_inited = false;
}

bool Manager::IsInitialized() const {
	return _inited;
}

void Manager::Update(DateTime dt) {
	if (!IsInitialized())
		return;

	for (auto& module : _modules) {
		module.Update(dt);
	}

	for (auto& plugin : _plugins) {
		if (plugin.GetState() == PluginState::Running) {
			plugin.GetModule()->UpdatePlugin(plugin, dt);
		}
	}
}

bool Manager::TopologicalSortPlugins() {
	if (_plugins.empty())
		return true; // Nothing to sort

	std::unordered_map<std::string, Plugin> pluginMap;
	std::unordered_map<std::string, size_t> inDegree;
	std::unordered_map<std::string, std::vector<std::string>> adjList;

	pluginMap.reserve(_plugins.size());
	inDegree.reserve(_plugins.size());
	adjList.reserve(_plugins.size());

	// Step 1: Build node map and initialize in-degrees
	for (Plugin& plugin : _plugins) {
		const std::string& name = plugin.GetName();
		inDegree.emplace(name, 0);
		pluginMap.emplace(name, std::move(plugin));
	}
	_plugins.clear();

	// Step 2: Build dependency graph
	for (const auto& [name, plugin] : pluginMap) {
		if (const auto& deps = plugin.GetManifest().dependencies) {
			for (const auto& dep : *deps) {
				if (pluginMap.contains(dep->name)) {
					adjList[dep->name].emplace_back(name);
					++inDegree[name];
				}
			}
		}
	}

	// Step 3: Collect plugins with no dependencies
	std::queue<std::string> ready;
	for (const auto& [name, degree] : inDegree) {
		if (degree == 0) {
			ready.push(name);
		}
	}

	// Step 4: Kahn's sort
	while (!ready.empty()) {
		const std::string current = std::move(ready.front());
		ready.pop();

		auto it = pluginMap.find(current);
		_plugins.emplace_back(std::move(std::get<Plugin>(*it)));
		pluginMap.erase(it);

		for (const auto& dependent : adjList[current]) {
			if (--inDegree[dependent] == 0) {
				ready.push(dependent);
			}
		}
	}

	// Step 5: Handle cyclic nodes
	if (!pluginMap.empty()) {
		std::vector<std::string_view> errors;

		for (auto& [name, plugin] : pluginMap) {
			errors.emplace_back(name);
		}

		if (!errors.empty()) {
			PL_LOG_WARNING("Plugin(s) '{}' is involved in a cyclic dependency and was excluded from the ordered load.", plg::join(errors, "', '"));
		}

		return false;  // Cycles were found
	}

	return true;  // No cycles
}

void Manager::DiscoverAllModulesAndPlugins() {
	PL_ASSERT(_allModules.empty() && "Modules already initialized");
	PL_ASSERT(_allPlugins.empty() && "Plugins already initialized");

	if (!PartitionLocalPackages())
		return;

	if (!TopologicalSortPlugins()) {
		PL_LOG_VERBOSE("Cyclic dependencies found during plugin sorting.");
	}

	PL_LOG_VERBOSE("Plugins order after topological sorting by dependency: ");
	for (const auto& plugin : _plugins) {
		PL_LOG_VERBOSE("--> {}", plugin.GetName());
	}
}

namespace {
    // Check conflicts for any item with a manifest
    template<typename Container, typename Item>
    std::vector<std::string> CheckConflicts(const Item& item, const Container& items) {
        std::vector<std::string> errors;

        if (const auto& conflicts = item.GetManifest().conflicts) {
            for (const auto& conflict : *conflicts) {
                auto it = std::ranges::find(items, conflict->name, &Item::GetName);

                if (it != items.end()) {
                    auto& version = it->GetManifest().version;

                    // Note: ConflictsWith returns a constraint when there IS a conflict
                    if (auto violatedConstraint = conflict->ConflictsWith(version)) {
                        std::string msg = std::format(
                            "Conflict: Incompatible with '{}' v{} (violates constraint: {}v{})",
                            conflict->name,
                            version,
							ComparisonUtils::ToString(violatedConstraint->comparison),
							violatedConstraint->version
                        );

                        if (!conflict->reason.empty()) {
                            std::format_to(std::back_inserter(msg), " - Reason: {}", conflict->reason);
                        }

                        errors.emplace_back(std::move(msg));
                    }
                }
            }
        }

        return errors;
    }

    // Check dependencies for any item with a manifest
    template<typename Container, typename Item>
    std::vector<std::string> CheckDependencies(
        const Item& item,
        const Container& items,
        std::string_view type)
    {
        std::vector<std::string> errors;

        if (const auto& dependencies = item.GetManifest().dependencies) {
            for (const auto& dependency : *dependencies) {
                auto it = std::ranges::find(items, dependency->name, &Item::GetName);

                if (it != items.end()) {
                    // Dependency exists but might not be loaded
                    if (it->GetState() != Item::State::Loaded) {
                        errors.emplace_back(std::format(
                            "Dependency: {} '{}' exists but is not loaded (state: {})",
                            type,
                            dependency->name,
                            Item::Utils::ToString(it->GetState())
                        ));

                        // Include the dependency's error if it has one
                        if (it->GetState() == Item::State::Error) {
                            errors.emplace_back(std::format(
                                "  └─ {}: {}",
                                dependency->name,
                                it->GetError()
                            ));
                        }
                    } else {
                        // Dependency is loaded, check version constraints
                        auto& foundVersion = it->GetManifest().version;

                        // Note: IsSatisfiedBy returns a constraint when NOT satisfied
                        if (auto unmetConstraint = dependency->IsSatisfiedBy(foundVersion)) {
                            errors.emplace_back(std::format(
                                "Dependency: {} '{}' version mismatch - found v{}, requires {}v{}",
                                type,
                                dependency->name,
                                foundVersion,
                                ComparisonUtils::ToString(unmetConstraint->comparison),
                                unmetConstraint->version
                            ));
                        }
                    }
                } else {
                    // Dependency doesn't exist
                    if (!dependency->optional.value_or(false)) {
                        errors.emplace_back(std::format(
                            "Dependency: Required {} '{}' is missing",
                            type,
                            dependency->name
                        ));
                    } else {
                        // Log optional dependency as info/debug rather than error
                        PL_LOG_DEBUG("Optional dependency '{}' for {} '{}' is not available",
                            dependency->name, type, item.GetName());
                    }
                }
            }
        }

        return errors;
    }

    // Combine all validation errors into a formatted message
    std::string FormatValidationErrors(const std::string& itemName,
                                       const std::vector<std::string>& errors) {
        if (errors.empty()) {
            return {};
        }

        std::string msg = std::format(
            "Failed to load '{}' due to {} issue(s):\n",
            itemName,
            errors.size()
        );

        for (size_t i = 0; i < errors.size(); ++i) {
            std::format_to(std::back_inserter(msg), "  {}. {}\n", i + 1, errors[i]);
        }

        return msg;
    }

	template<typename ItemType>
	std::vector<std::string> Validate(const ItemType& item,
					const auto& container,
					std::string_view type) {
    	std::vector<std::string> errors;

    	// Check conflicts
    	auto conflictErrors = CheckConflicts(item, container);
    	errors.insert(errors.end(), conflictErrors.begin(), conflictErrors.end());

    	// Check dependencies
    	auto dependencyErrors = CheckDependencies(item, container, type);
    	errors.insert(errors.end(), dependencyErrors.begin(), dependencyErrors.end());

    	return errors;
    }

	template <typename T>
	Result<std::shared_ptr<Manifest>> ReadManifest(
		const std::shared_ptr<IFileSystem>& fs,
		const fs::path& path,
		ManifestType type
	) {
		auto json = fs->ReadTextFile(path);
		if (!json)
			return plg::unexpected(json.error());

		auto parsed = glz::read_jsonc<std::shared_ptr<T>>(*json);
		if (!parsed)
			return plg::unexpected(glz::format_error(parsed.error(), *json));

		auto& manifest = *parsed;
		manifest->path = path;
		manifest->type = type;

		if (!SupportsPlatform(manifest->platforms))
			return plg::unexpected(std::string()); // supress

		auto errors = manifest->Validate();
		if (!errors.empty())
			return plg::unexpected(plg::join(errors, ", "));

		return std::static_pointer_cast<Manifest>(std::move(manifest));
	}

	void HandleDuplicateManifest(
		ManifestList& manifests,
		std::shared_ptr<Manifest> manifest)
	{
		auto it = std::ranges::find(manifests, manifest->name, &Manifest::name);

		if (it == manifests.end()) {
			// No duplicate - add new manifest
			manifests.emplace_back(std::move(manifest));
			return;
		}

		// Handle duplicate
		auto& existing = *it;
		const auto& oldVer = existing->version;
		const auto& newVer = manifest->version;

		if (oldVer == newVer) {
			PL_LOG_WARNING(
				"Duplicate package '{}' v{} found at '{}' - ignoring second location",
				manifest->name, oldVer, manifest->path.string()
			);
		} else {
			// Keep newer version
			const bool useNew = newVer > oldVer;
			PL_LOG_WARNING(
				"Multiple versions of '{}' found - using v{} (discarding v{})",
				manifest->name,
				useNew ? newVer : oldVer,
				useNew ? oldVer : newVer
			);

			if (useNew) {
				existing = std::move(manifest);
			}
		}
	}

	template<typename T>
	bool ProcessManifestDirectory(
		ManifestList& manifests,
		const std::shared_ptr<IFileSystem>& fs,
		const fs::path& dirPath,
		std::string_view pattern,
		ManifestType type)
	{
		// Find manifest files
		auto found = fs->FindFiles(dirPath, {pattern}, false);
		if (!found) {
			PL_LOG_ERROR("Failed to read directory '{}': {}",
						 dirPath.string(), found.error());
			return false;
		}

		if (found->empty())
			return false;

		if (found->size() > 1) {
			PL_LOG_WARNING("Directory '{}' contains {} manifest files - using first",
						   dirPath.string(), found->size());
		}

		// Read and process manifest
		auto result = ReadManifest<T>(fs, found->front(), type);
		if (!result) {
			if (!result.error().empty()) {
				PL_LOG_ERROR("Failed to load manifest from '{}': {}",
							dirPath.string(), result.error());
			}
			return false;
		}

		HandleDuplicateManifest(manifests, std::move(*result));
		return true;
	}
}

ManifestList Manager::FindLocalPackages() {
	auto fs = _plugify.GetFileSystem();
	auto& config = _plugify.GetConfig().baseDir;

	auto entries = fs->IterateDirectory(config, {false});
	if (!entries) {
		PL_LOG_ERROR("Failed to read base directory '{}': {}",
					config.string(), entries.error());
		return {};
	}

	ManifestList manifests;

	for (const auto& entry : *entries) {
		const auto& [path, lastModified, size, isDirectory, isRegularFile, isSymlink] = entry;
		if (!isDirectory) {
			continue;
		}

		// Process language modules
		if (ProcessManifestDirectory<ModuleManifest>(
			manifests, fs, path, Module::kFileExtension, ManifestType::Module)
		) {
			continue;
		}

		// Process plugins
		if (ProcessManifestDirectory<PluginManifest>(
			manifests, fs, path, Plugin::kFileExtension, ManifestType::Plugin)
		) {
			continue;
		}

		PL_LOG_VERBOSE("Directory with no manifest: '{}'", path.string());
	}

	return manifests;
}

bool Manager::PartitionLocalPackages() {
	auto manifests = FindLocalPackages();
	if (manifests.empty()) {
		PL_LOG_WARNING("Did not find any package.");
		return false;
	}

	auto count = manifests.size();
	auto pluginCount = static_cast<size_t>(std::ranges::count(manifests, ManifestType::Plugin, &Manifest::type));
	_plugins.reserve(pluginCount);
	_modules.reserve(count - pluginCount);

	const auto& config = _plugify.GetConfig();
	for (auto& manifest : manifests) {
		BasePaths paths {
				.base = manifest->path.parent_path(),
				.configs = config.baseDir / config.configsDir,
				.data = config.baseDir / config.dataDir,
				.logs = config.baseDir / config.logsDir,
		};

		if (manifest->type == ManifestType::Plugin) {
			_plugins.emplace_back(static_cast<UniqueId>(_plugins.size()), std::move(paths), std::move(manifest));
		} else {
			_modules.emplace_back(static_cast<UniqueId>(_modules.size()), std::move(paths), std::move(manifest));
		}
	}

	if (_modules.empty()) {
		PL_LOG_WARNING("Did not find any module.");
		return false;
	}

	if (_plugins.empty()) {
		PL_LOG_WARNING("Did not find any plugin.");
		return false;
	}

	return true;
}

void Manager::LoadRequiredLanguageModules() {
	if (_modules.empty())
		return;

	std::unordered_set<UniqueId> modules;
	modules.reserve(_modules.size());

	for (auto& plugin : _plugins) {
		const auto& [name, constraits, _] = plugin.GetManifest().language;
		auto it = std::ranges::find(_modules, name, &Module::GetLanguage);
		if (it == _modules.end()) {
			plugin.SetError(std::format("Language module: '{}' missing for plugin: '{}'", name, plugin.GetName()));
			continue;
		} else {
			if (constraits) {
				auto& version = it->GetManifest().version;
				for (const auto& constraint : *constraits) {
					if (!constraint.IsSatisfiedBy(version)) {
						std::string errorMsg = std::format(
							"Plugin '{}' requrement version mismatch - found v{}, requires {}v{}",
							plugin.GetName(), dependency.name
						);

						for (const auto& failure : validation.failedConstraints) {
							errorMsg += std::format("  - {}\n", failure);
						}

						plugin.SetError(errorMsg);
					}
				}
			}
		}

		auto& module = *it;
		plugin.Initialize(_plugify);
		plugin.SetModule(module);
		modules.emplace(module.GetId());
	}

	bool loadedAny = false;

	for (auto& module : _modules) {
		if (module.GetManifest().forceLoad.value_or(false) || modules.contains(module.GetId())) {
			// Perform conflict and dependency checks for modules
			if (module.GetState() == ModuleState::NotLoaded) {
				auto errors = Validate(module, _modules, "Module");
				if (!errors.empty()) {
					module.SetError(FormatValidationErrors(module.GetName(), errors));
				} else {
					loadedAny |= module.Initialize(_plugify);
				}
			}
		}
	}
	
	if (!loadedAny) {
		PL_LOG_WARNING("Did not load any module");
		return;
	}
}

void Manager::LoadAndStartAvailablePlugins() {
	if (_plugins.empty())
		return;
	
	bool loadedAny = false;

	for (auto& plugin : _plugins) {
		if (plugin.GetState() == PluginState::NotLoaded) {
			if (plugin.GetModule()->GetState() != ModuleState::Loaded) {
				plugin.SetError(std::format("Language module '{}' is not loaded", plugin.GetModule()->GetName()));
				continue;
			}

			auto errors = Validate(plugin, _plugins, "Plugin");
			if (!errors.empty()) {
				plugin.SetError(FormatValidationErrors(plugin.GetName(), errors));
			} else {
				loadedAny |= plugin.GetModule()->LoadPlugin(plugin);
			}
		}
	}
	
	if (!loadedAny) {
		PL_LOG_WARNING("Did not load any plugin");
		return;
	}

	for (auto& plugin : _plugins) {
		if (plugin.GetState() == PluginState::Loaded) {
			for (const auto& module : _modules) {
				module.MethodExport(plugin);
			}
		}
	}

	for (auto& plugin : _plugins) {
		if (plugin.GetState() == PluginState::Loaded) {
			plugin.GetModule()->StartPlugin(plugin);
		}
	}
}

void Manager::TerminateAllPlugins() {
	if (_plugins.empty())
		return;
	
	for (auto& plugin : std::ranges::reverse_view(_plugins)) {
		if (plugin.GetState() == PluginState::Running) {
			plugin.GetModule()->EndPlugin(plugin);
		}
	}

	for (auto& plugin : std::ranges::reverse_view(_plugins)) {
		plugin.Terminate();
	}

	_plugins.clear();
}

void Manager::TerminateAllModules() {
	if (_modules.empty())
		return;

	for (auto& module : std::ranges::reverse_view(_modules)) {
		module.Terminate();
	}

	_modules.clear();
}

ModuleHandle Manager::FindModule(std::string_view moduleName) const {
	auto it = std::ranges::find(_modules, moduleName, &Module::GetName);
	if (it != _modules.end())
		return *it;
	return {};
}

ModuleHandle Manager::FindModuleFromId(UniqueId moduleId) const {
	auto it = std::ranges::find(_modules, moduleId, &Module::GetId);
	if (it != _modules.end())
		return *it;
	return {};
}

std::vector<ModuleHandle> Manager::GetModules() const {
	std::vector<ModuleHandle> modules;
	modules.reserve(_modules.size());
	for (const auto& module : _modules)  {
		modules.emplace_back(module);
	}
	return modules;
}

PluginHandle Manager::FindPlugin(std::string_view pluginName) const {
	auto it = std::ranges::find(_plugins, pluginName, &Plugin::GetName);
	if (it != _plugins.end())
		return *it;
	return {};
}

PluginHandle Manager::FindPluginFromId(UniqueId pluginId) const {
	auto it = std::ranges::find(_plugins, pluginId, &Plugin::GetId);
	if (it != _plugins.end())
		return *it;
	return {};
}

std::vector<PluginHandle> Manager::GetPlugins() const {
	std::vector<PluginHandle> plugins;
	plugins.reserve(_plugins.size());
	for (const auto& plugin : _plugins)  {
		plugins.emplace_back(plugin);
	}
	return plugins;
}
