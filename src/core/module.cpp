#include "plugify/core/module.hpp"
#include "plugify/core/assembly.hpp"
#include "plugify/core/language_module.hpp"
#include "plugify/core/conflict.hpp"
#include "plugify/core/dependency.hpp"
#include "plugify/core/method.hpp"

#if 0
#include "plugify/_/module_handle.hpp"
#include "plugify/_/provider_handle.hpp"
#include "plugify/core/plugify.hpp"
#include "plugify/asm/mem_protector.hpp"
#include "plugify/core/plugin.hpp"
#include "plugify/core/provider.hpp"
using namespace plugify;

Module::Module(UniqueId id, BasePaths paths, std::shared_ptr<Manifest> manifest)
	: _id{id}, _paths{std::move(paths)}, _manifest{std::static_pointer_cast<ModuleManifest>(std::move(manifest))} {
	PL_ASSERT(_manifest->type != PackageType::Plugin && "Invalid package type for module ctor");
	PL_ASSERT(_manifest->path.has_parent_path() && "Package path doesn't contain parent path");
}

Module::Module(Module&& module) noexcept {
	*this = std::move(module);
}

bool Module::Initialize(Plugify& plugify) {
	PL_ASSERT(GetState() != ModuleState::Loaded && "Module already was initialized");

	// Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.
	auto filePath = _paths.base / "bin" / std::format(PLUGIFY_LIBRARY_PREFIX "{}" PLUGIFY_LIBRARY_SUFFIX, _manifest->name);

	auto res = loader->Load(filePath, flags);
	if (!res) {
		SetError(std::format("Failed to load library: '{}' at: '{}' - {}", GetName(), filePath.string(), res.error()));
		return false;
	}

	auto& assembly = *res;

	auto GetLanguageModuleFunc = assembly->GetSymbol(kGetLanguageModuleFn).RCast<ILanguageModule*(*)()>();
	if (!GetLanguageModuleFunc) {
		SetError(std::format("Function '{}' not exist inside '{}' library", kGetLanguageModuleFn, filePath.string()));
		Terminate();
		return false;
	}

	auto* languageModule = GetLanguageModuleFunc();
	if (!languageModule) {
		SetError(std::format("Function '{}' inside '{}' library. Returned invalid address of 'ILanguageModule' implementation!", kGetLanguageModuleFn, filePath.string()));
		Terminate();
		return false;
	}

#if PLUGIFY_PLATFORM_WINDOWS
	constexpr bool plugifyBuildType = PLUGIFY_IS_DEBUG;
	bool moduleBuildType = languageModule->IsDebugBuild();
	if (moduleBuildType != plugifyBuildType) {
		SetError(std::format("Mismatch between plugify ({}) build type and module ({}) build type.", (plugifyBuildType ? "debug" : "release"), (moduleBuildType ? "debug" : "release"))); //-V547
		Terminate();
		return false;
	}
#endif // PLUGIFY_PLATFORM_WINDOWS

	auto result = languageModule->Initialize(plugify.GetProvider(), *this);
	if (!result) {
		SetError(std::format("Failed to initialize module: '{}' error: '{}' at: '{}'", GetName(), result.error(), filePath.string()));
		Terminate();
		return false;
	}

	_assembly = std::move(assembly);
	_languageModule = languageModule;
	_table = result->table;

	SetLoaded();
	return true;
}

void Module::Terminate() {
	if (_languageModule) {
		_languageModule->Shutdown();
		_languageModule = nullptr;
	}
	_assembly.reset();

	_loadedPlugins.clear();

	SetUnloaded();
}

void Module::Update(DateTime dt) {
	if (_languageModule && _table.hasUpdate) {
		_languageModule->OnUpdate(dt);
	}
}

bool Module::LoadPlugin(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return false;

	auto result = _languageModule->OnPluginLoad(plugin);
	if (!result) {
		plugin.SetError(std::format("Failed to load plugin: '{}' error: '{}' at: '{}'", plugin.GetName(), result.error(), plugin.GetBaseDir().string()));
		return false;
	}

	if (const auto& methods = plugin.GetManifest().methods) {
		if (result->methods.size() != methods->size()) {
			plugin.SetError(std::format("Mismatch in methods count, expected: {} but provided: {}", methods->size(), result->methods.size()));
			return false;
		}

		std::vector<std::string_view> errors;

		for (size_t i = 0; i < result->methods.size(); ++i) {
			const auto& [method, addr] = result->methods[i];
			const auto& exportedMethod = (*methods)[i];

			if (method != *exportedMethod || !addr) {
				errors.emplace_back(exportedMethod->name);
			}
		}

		if (!errors.empty()) {
			plugin.SetError(std::format("Found invalid {} method(s)", plg::join(errors, ", ")));
			return false;
		}

		plugin.SetMethods(std::move(result->methods));
	}

	plugin.SetTable(result->table);
	plugin.SetData(result->data);

	plugin.SetLoaded();

	_loadedPlugins.emplace_back(plugin);

	return true;
}

void Module::MethodExport(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasExport()) {
		_languageModule->OnMethodExport(plugin);
	}
}

void Module::StartPlugin(Plugin& plugin) const  {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasStart()) {
		_languageModule->OnPluginStart(plugin);
	}

	plugin.SetRunning();
}

void Module::UpdatePlugin(Plugin& plugin, DateTime dt) const  {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasUpdate()) {
		_languageModule->OnPluginUpdate(plugin, dt);
	}
}

void Module::EndPlugin(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasEnd()) {
		_languageModule->OnPluginEnd(plugin);
	}

	plugin.SetTerminating();
}

void Module::SetError(std::string error) {
	PL_ASSERT(error.empty() && "Empty error string!");
	_error = std::move(error);
	_state = ModuleState::Error;
	PL_LOG_ERROR("Module '{}': {}", GetName(), GetError());
}
#endif

using namespace plugify;

// Internal state
struct Module::Impl {
    UniqueId id;
    ILanguageModule* _languageModule{ nullptr };
    MethodTable _table;
    std::shared_ptr<IAssembly> _assembly;
    std::shared_ptr<ModuleManifest> manifest;
    BasePaths paths;
    bool initialized{ false };
};

// ctor/dtor/copy/move
Module::Module(UniqueId id, ManifestPtr manifest) : _impl(std::make_unique<Impl>()) {
    PL_ASSERT(manifest->type == PackageType::Module && "Invalid package type for module ctor");
    PL_ASSERT(manifest->path.has_parent_path() && "Package path doesn't contain parent path");
    _impl->id = id;
    _impl->manifest = std::static_pointer_cast<ModuleManifest>(std::move(manifest));
}

Module::~Module() = default;

Module::Module(const Module& other)
    : _impl(std::make_unique<Impl>(*other._impl)) {}

Module::Module(Module&& other) noexcept = default;

Module& Module::operator=(const Module& other) {
    if (this != &other) {
        _impl = std::make_unique<Impl>(*other._impl);
    }
    return *this;
}

Module& Module::operator=(Module&& other) noexcept = default;

// Lifecycle
Result<void> Module::Initialize() {
    PL_ASSERT(_impl->initialized && "Module already initialized");
    _impl->initialized = true;
}

Result<void> Module::Load() {
    PL_ASSERT(!_impl->initialized && "Module not initialized");

}

void Module::Update() {
    PL_ASSERT(!_impl->initialized && "Module not initialized");
}

void Module::Unload() {
    PL_ASSERT(!_impl->initialized && "Module not initialized");
}

void Module::Terminate() {
    PL_ASSERT(!_impl->initialized && "Module not initialized");
    _impl->initialized = false;
}

// Plugin interaction
bool Module::LoadPlugin(Plugin& plugin) const {
    // TODO: load plugin's code/resources into this module
    return true;
}

void Module::StartPlugin(Plugin& plugin) const {
    // TODO: start plugin execution
}

void Module::UpdatePlugin(Plugin& plugin, DateTime dt) const {
    // TODO: update plugin with current time
    (void)plugin;
    (void)dt;
}

void Module::EndPlugin(Plugin& plugin) const {
    // TODO: safely shutdown plugin
}

void Module::MethodExport(Plugin& plugin) const {
    // TODO: expose plugin methods to runtime
}

// Default empty containers
static std::string emptyString;
static std::filesystem::path emptyPath;
static std::vector<std::string> emptyStrings;
static std::vector<Dependency> emptyDependencies;
static std::vector<Conflict> emptyConflicts;
static std::vector<Obsolete> emptyObsoletes;

// Getters
const UniqueId& Module::GetId() const noexcept { return _impl->id; }
const std::string& Module::GetName() const noexcept { return _impl->manifest->name; }
PackageType Module::GetType() const noexcept { return _impl->manifest->type; }
const Version& Module::GetVersion() const noexcept { return _impl->manifest->version; }
const std::string& Module::GetDescription() const noexcept { return _impl->manifest->description ? *_impl->manifest->description : emptyString; }
const std::string& Module::GetAuthor() const noexcept { return _impl->manifest->author ? *_impl->manifest->author : emptyString; }
const std::string& Module::GetWebsite() const noexcept { return _impl->manifest->website ? *_impl->manifest->website : emptyString; }
const std::string& Module::GetLicense() const noexcept { return _impl->manifest->license ? *_impl->manifest->license : emptyString; }
const std::filesystem::path& Module::GetLocation() const noexcept { return _impl->manifest->path; }
const std::vector<std::string> Module::GetPlatforms() const noexcept { return _impl->manifest->platforms ? *_impl->manifest->platforms : emptyStrings; }
const std::vector<Dependency> Module::GetDependencies() const noexcept { return _impl->manifest->dependencies ? *_impl->manifest->dependencies : emptyDependencies; }
const std::vector<Conflict> Module::GetConflicts() const noexcept { return _impl->manifest->conflicts ? *_impl->manifest->conflicts : emptyConflicts; }
const std::vector<Obsolete> Module::GetObsoletes() const noexcept { return _impl->manifest->obsoletes ? *_impl->manifest->obsoletes : emptyObsoletes; }

const std::string& Module::GetLanguage() const noexcept { return _impl->manifest->language; }
const std::filesystem::path& Module::GetRuntime() const noexcept { return _impl->manifest->runtime ? *_impl->manifest->runtime : emptyPath; }
const std::vector<std::string> Module::GetDirectories() const noexcept { return _impl->manifest->directories ? *_impl->manifest->directories : emptyStrings; }
bool Module::GetForceLoad() const noexcept { return _impl->manifest->forceLoad.value_or(false); }

const std::filesystem::path& Module::GetBaseDir() const noexcept { return _impl->paths.base; }
const std::filesystem::path& Module::GetConfigsDir() const noexcept { return _impl->paths.configs; }
const std::filesystem::path& Module::GetDataDir() const noexcept { return _impl->paths.data; }
const std::filesystem::path& Module::GetLogsDir() const noexcept { return _impl->paths.logs; }

// Setters
void Module::SetId(UniqueId id) noexcept { _impl->id = id; }
void Module::SetName(std::string name) noexcept { _impl->manifest->name = std::move(name); }
void Module::SetType(PackageType type) noexcept { _impl->manifest->type = type; }
void Module::SetVersion(Version version) noexcept { _impl->manifest->version = std::move(version); }
void Module::SetDescription(std::string description) noexcept { _impl->manifest->description = std::move(description); }
void Module::SetAuthor(std::string author) noexcept { _impl->manifest->author = std::move(author); }
void Module::SetWebsite(std::string website) noexcept { _impl->manifest->website = std::move(website); }
void Module::SetLicense(std::string license) noexcept { _impl->manifest->license = std::move(license); }
void Module::SetLocation(std::filesystem::path location) noexcept { _impl->manifest->path = std::move(location); }
void Module::SetPlatforms(std::vector<std::string> platforms) noexcept { _impl->manifest->platforms = std::move(platforms); }
void Module::SetDependencies(std::vector<Dependency> dependencies) noexcept { _impl->manifest->dependencies = std::move(dependencies); }
void Module::SetConflicts(std::vector<Conflict> conflicts) noexcept { _impl->manifest->conflicts = std::move(conflicts); }
void Module::SetObsoletes(std::vector<Obsolete> obsoletes) noexcept { _impl->manifest->obsoletes = std::move(obsoletes); }
void Module::SetLanguage(std::string language) noexcept { _impl->manifest->language = std::move(language); }
void Module::SetRuntime(std::filesystem::path runtimePath) noexcept { _impl->manifest->runtime = std::move(runtimePath); }
void Module::SetDirectories(std::vector<std::string> directories) noexcept { _impl->manifest->directories = std::move(directories); }
void Module::SetForceLoad(bool forceLoad) noexcept { _impl->manifest->forceLoad = forceLoad; }

void Module::SetBaseDir(std::filesystem::path base) noexcept { _impl->paths.base = std::move(base); }
void Module::SetConfigsDir(std::filesystem::path configs) noexcept { _impl->paths.configs = std::move(configs); }
void Module::SetDataDir(std::filesystem::path data) noexcept { _impl->paths.data = std::move(data); }
void Module::SetLogsDir(std::filesystem::path logs) noexcept { _impl->paths.logs = std::move(logs); }

// Comparison
bool Module::operator==(const Module& other) const noexcept = default;
auto Module::operator<=>(const Module& other) const noexcept = default;