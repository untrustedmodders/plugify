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
	PL_ASSERT(_modules.empty() && "Modules already initialized");
	PL_ASSERT(_plugins.empty() && "Plugins already initialized");

	if (!PartitionLocalPackages())
		return;

	if (!TopologicalSortPlugins()) {
		PL_LOG_VERBOSE("Cyclic dependencies found during plugin sorting.");
	}

	PL_LOG_VERBOSE("Plugins order after topological sorting by dependency: ");
	for (const auto& plugin : _plugins) {
		PL_LOG_VERBOSE("  └─ {}", plugin.GetName());
	}
}

namespace {
	bool SupportsPlatform(const std::optional<std::vector<std::string>>& supportedPlatforms) {
		if (!supportedPlatforms || supportedPlatforms->empty())
			return true;

		constexpr std::string_view platform = PLUGIFY_PLATFORM; // e.g., "linux_x64"

		constexpr auto separator_pos = platform.find('_');
		static_assert(separator_pos != std::string_view::npos,
					  "PLUGIFY_PLATFORM must be in the format 'os_arch'");

		constexpr std::string_view os = platform.substr(0, separator_pos);
		constexpr std::string_view arch = platform.substr(separator_pos + 1);

		return std::ranges::any_of(*supportedPlatforms, [&](const std::string& supported) {
			// Exact match
			if (supported == platform)
				return true;

			// Wildcard support: "linux_*" matches any Linux, "*_x64" matches any x64
			if (supported.ends_with("_*")) {
				return supported.substr(0, supported.size() - 2) == os;
			}
			if (supported.starts_with("*_")) {
				return supported.substr(2) == arch;
			}
			if (supported == "*" || supported == "*_*") {
				return true; // Matches all platforms
			}

			return false;
		});
	}

	struct ValidationResult {
		std::vector<std::pair<std::string_view, std::vector<Constraint>>> failedDependencies;
		std::vector<std::pair<std::string_view, std::vector<Constraint>>> detectedConflicts;
		std::vector<std::string_view> conflictReasons;

		size_t TotalIssues() const {
			return  failedDependencies.size() +
					detectedConflicts.size() +
					conflictReasons.size();
		}

		operator bool() const {
			return failedDependencies.empty() &&
				   detectedConflicts.empty() &&
				   conflictReasons.empty();
		}
	};

	template<typename Item, typename Container>
	ValidationResult ValidateAgainst(
		const Item& item,
		const Container& container
	) {
		ValidationResult result;

		// Dependencies
		for (const auto& dep : item.GetDependencies()) {
			auto it = std::ranges::find_if(container, [&](const Item& i) {
				return i.GetName() == dep->name && i.GetName() != item.GetName();
			});
			if (it == container.end()) {
				if (!dep->optional || !*dep->optional) {
					result.failedDependencies.emplace_back(dep->name, dep->constraints.value_or(std::vector<Constraint>{}));
				}
			} else {
				auto failed = dep->GetFailedConstraints(it->GetVersion());
				if (!failed.empty() && (!dep->optional || !*dep->optional)) {
					result.failedDependencies.emplace_back(dep->name, failed);
				}
			}
		}

		// Conflicts
		for (const auto& conflict : item.GetConflicts()) {
			auto it = std::ranges::find_if(container, [&](const Item& i) {
				return i.GetName() == conflict->name && i.GetName() != item.GetName();
			});
			if (it != container.end()) {
				auto satisfied = conflict->GetSatisfiedConstraints(it->GetVersion());
				result.detectedConflicts.emplace_back(conflict->name, satisfied);
				if (conflict->reason) result.conflictReasons.emplace_back(*conflict->reason);
			}
		}

		return result;
    }

	template<typename Item, typename Container>
	void ShowValidationReport(
		const Item& item,
		const Container& container,
		const ValidationResult& result
	) {
		LOG("", Color::None);
		LOG("Validation Report for {} '{}' v{}:", Color::Bold,
			item.GetType(), item.GetName(), item.GetVersion());

		if (result) {
			LOG("  [✓] All checks passed", Color::Green);
			return;
		}

		if (!result.failedDependencies.empty()) {
			LOG("  Failed Dependencies:", Color::Red);
			for (const auto& [name, failed] : result.failedDependencies) {
				auto it = std::ranges::find(container, name, &Item::GetName);
				if (it == container.end()) {
					LOG("    - '{}' [missing]", Color::Red, name);
				} else {
					LOG("    - '{}' v{} did not satisfy constraints:", Color::Red,
						name, it->GetVersion());
					for (const auto& c : failed) {
						LOG("       ✗ {}", Color::Red, c);
					}
				}
			}
		}

		if (!result.detectedConflicts.empty()) {
			LOG("  Detected Conflicts:", Color::Red);
			for (const auto& [name, satisfied] : result.detectedConflicts) {
				auto it = std::ranges::find(container, name, &Item::GetName);
				if (it != container.end()) {
					LOG("    - '{}' v{} conflicts:", Color::Red,
						name, it->GetVersion());
				} else {
					LOG("    - '{}' [present?] conflicts:", Color::Red, name);
				}

				for (const auto& c : satisfied) {
					LOG("       ⚠ {}", Color::Yellow, c);
				}
			}
		}

		if (!result.conflictReasons.empty()) {
			LOG("  Conflict Reasons:", Color::Yellow);
			for (const auto& reason : result.conflictReasons) {
				LOG("    - {}", Color::Yellow, reason);
			}
		}

		LOG("", Color::None);
	}
	
	template<typename Item, typename Container>
	bool Validate(
		const Item& item,
		const Container& container,
		bool verbose = false
	) {
		auto result = ValidateAgainst(item, container);
    
		if (result) {
			if (verbose) {
				LOG("[✓] {} '{}' v{} - All checks passed", Color::Green,
					item.GetType(), item.GetName(), item.GetVersion());
				LOG("    {} dependencies satisfied, {} conflicts checked", Color::Gray,
					item.GetDependencies().size(), item.GetConflicts().size());
			}
			return true;
		} else {
			ShowValidationReport(item, container, result);
			return false;
		}
	}

	template <typename T>
	Result<std::shared_ptr<Manifest>> ReadManifest(
		const std::shared_ptr<IFileSystem>& fs,
		const fs::path& path,
		ManifestType type
	) {
		auto json = fs->ReadTextFile(path);
		if (!json)
			return std::unexpected(json.error());

		auto parsed = glz::read_jsonc<std::shared_ptr<T>>(*json);
		if (!parsed)
			return std::unexpected(glz::format_error(parsed.error(), *json));

		auto& manifest = *parsed;
		manifest->path = path;
		manifest->type = type;

		return std::static_pointer_cast<Manifest>(std::move(manifest));
	}

	void HandleDuplicateManifest(
		ManifestList& manifests,
		std::shared_ptr<Manifest> manifest
	) {
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
		ManifestType type
	) {
		// Find manifest files
		auto paths = fs->FindFiles(dirPath, {pattern}, false);
		if (!paths) {
			PL_LOG_ERROR("Failed to read directory '{}': {}",
						 dirPath.string(), paths.error());
			return false;
		}

		if (paths->empty())
			return false;

		if (paths->size() > 1) {
			PL_LOG_WARNING("Directory '{}' contains {} manifest files - using first",
						   dirPath.string(), paths->size());
		}

		// Read and process manifest
		auto result = ReadManifest<T>(fs, paths->front(), type);
		if (!result) {
			PL_LOG_ERROR("Failed to load manifest from '{}': {}",
							dirPath.string(), result.error());
			return false;
		}

    	auto& manifest = *result;

		// Skip manifest which not valid arch
    	if (!SupportsPlatform(manifest->platforms))
    		return false;

		ScopeLog scope{std::format("Failed to validate manifest from '{}':", dirPath.string())};
    	manifest->Validate(scope);

		if (scope) {
			PL_LOG_VERBOSE("Detect {} fails", scope.count);
			return false;
		}

		HandleDuplicateManifest(manifests, std::move(manifest));
		return true;
	}
}

ManifestList Manager::FindLocalPackages() {
	auto fs = _plugify.GetFileSystem();
	auto& baseDir = _plugify.GetConfig().baseDir;

	auto entries = fs->IterateDirectory(baseDir, {false});
	if (!entries) {
		PL_LOG_ERROR("Failed to read base directory '{}': {}",
					baseDir.string(), entries.error());
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
/*
struct ScopedResult {
	std::string_view type;
	size_t succeeded = 0;
	size_t failed = 0;

	~ScopedResult() {
		if (HasAny()) {
			PL_LOG_INFO("{} initialization complete: {} succeeded, {} failed",
					   type, succeeded, failed);
		} else {
			PL_LOG_WARNING("Did not initialize any {}", type);
		}
	}

	bool HasAny() const { return succeeded > 0 || failed > 0; }
};

// Process single plugin - returns module ID if successful
std::optional<UniqueId> Manager::ProcessPluginInitialization(
	Plugin& plugin,
	ScopedResult& result) {

	// Early return for missing module
	const auto& [name, constraints, _] = plugin.GetManifest().language;
	auto module = std::ranges::find(_modules, name, &Module::GetLanguage);
	if (module != _modules.end()) {
		plugin.SetError(FormatMissingModuleError(plugin));
		++result.failed;
		return std::nullopt;
	}

	// Early return for constraint violations
	if (constraints) {

	}
	auto errors = ValidateConstraints(*module, *constraints);
	if (!errors.empty()) {
		plugin.SetError(*error);
		++result.failed;
		return std::nullopt;
	}

	// Early return for initialization failure
	if (!plugin.Initialize(_plugify)) {
		++result.failed;
		return std::nullopt;
	}

	// Success path
	plugin.SetModule(*module);
	++result.succeeded;
	return module->GetId();
}

std::unordered_set<UniqueId> Manager::InitializePlugins() {
	std::unordered_set<UniqueId> requiredModuleIds;
	requiredModuleIds.reserve(_modules.size());

	ScopedResult result{"plugin"};

	for (auto& plugin : _plugins) {
		if (auto moduleId = ProcessPluginInitialization(plugin, result)) {
			requiredModuleIds.insert(*moduleId);
		}
	}

	return requiredModuleIds;
}
*/
void Manager::LoadRequiredLanguageModules() {
	if (_modules.empty())
		return;

	/*std::unordered_set<UniqueId> modules;
	modules.reserve(_modules.size());

	size_t successCount = 0;
	size_t failedCount = 0;

	for (auto& plugin : _plugins) {
		const auto& [name, constraits, _] = plugin.GetManifest().language;
		auto it = std::ranges::find(_modules, name, &Module::GetLanguage);
		if (it == _modules.end()) {
			plugin.SetError(std::format("Language module: '{}' missing for plugin: '{}'", name, plugin.GetName()));
			++failedCount;
			continue;
		}

		if (constraits) {
			auto errors = ValidateConstraints(*it, *constraits);
			if (!errors.empty()) {
				std::string msg = std::format(
					"Plugin '{}' requires module '{}' with unsatisfied constraints:\n",
					plugin.GetName(), name
				);
				for (const auto& failure : errors) {
					std::format_to(std::back_inserter(msg), "  - {}\n", failure);
				}
				plugin.SetError(std::move(msg));
				++failedCount;
				continue;
			}
		}

		if (plugin.Initialize(_plugify)) {
			++successCount;

			auto& module = *it;
			plugin.SetModule(module);
			modules.emplace(module.GetId());
		}
	}

	if (successCount || failedCount) {
		PL_LOG_INFO("Plugin initialization complete: {} succeeded, {} failed",
						successCount, failedCount);
	} else {
		PL_LOG_WARNING("Did not initialize any plugin");
	}

	successCount = 0;
	failedCount = 0;

	for (auto& module : _modules) {
		if (module.GetManifest().forceLoad.value_or(false) || modules.contains(module.GetId())) {
			// Perform conflict and dependency checks for modules
			if (module.GetState() == ModuleState::NotLoaded) {
				auto errors = Validate(module, _modules, "Module");
				if (!errors.empty()) {
					module.SetError(FormatValidationErrors(module.GetName(), errors));
				} else {
					if (module.Initialize(_plugify)) {
						++successCount;
					}
				}
			}
		}
	}

	if (successCount || failedCount) {
		PL_LOG_INFO("Module initialization complete: {} succeeded, {} failed",
						successCount, failedCount);
	} else {
		PL_LOG_WARNING("Did not initialize any module");
	}*/
	auto requiredModuleIds = InitializePluginsWithModules();
	LoadModules(requiredModuleIds);
}

void Manager::LoadAndStartAvailablePlugins() {
	if (_plugins.empty())
		return;

	size_t successCount = 0;
	size_t failedCount = 0;

	for (auto& plugin : _plugins) {
		if (plugin.GetState() == PluginState::NotLoaded) {
			if (plugin.GetModule()->GetState() != ModuleState::Loaded) {
				plugin.SetError(std::format("Language module '{}' is not loaded", plugin.GetModule()->GetName()));
				continue;
			}

			if (!Validate(plugin, _plugins, true)) {
				plugin.SetError(FormatValidationErrors(plugin.GetName(), errors));
				continue;
			}

			if (plugin.GetModule()->LoadPlugin(plugin)) {
				++successCount;
			} else {
				++failedCount;
			}
		}
	}

	if (successCount || failedCount) {
		PL_LOG_INFO("Plugins loading complete: {} succeeded, {} failed", successCount, failedCount);
	} else {
		PL_LOG_WARNING("Did not load any plugin");
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
	
	for (auto& plugin : _plugins | std::views::reverse) {
		if (plugin.GetState() == PluginState::Running) {
			plugin.GetModule()->EndPlugin(plugin);
		}
	}

	for (auto& plugin : _plugins | std::views::reverse) {
		plugin.Terminate();
	}

	_plugins.clear();
}

void Manager::TerminateAllModules() {
	if (_modules.empty())
		return;

	for (auto& module : _modules | std::views::reverse) {
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
