#include "plugify/core/conflict.hpp"
#include "plugify/core/dependency.hpp"
#include "plugify/core/language_module.hpp"
#include "plugify/core/method.hpp"
#include "plugify/core/module.hpp"
#include "plugify/core/plugin.hpp"

using namespace plugify;

// Plugin Implementation

struct Plugin::Impl {
    UniqueId id;
    ILanguageModule* languageModule{ nullptr };
    PluginState state{ PluginState::Unloaded };
    MethodTable table;
    MemAddr userData;
	std::shared_ptr<Module> module;
	std::shared_ptr<PluginManifest> manifest;
    std::vector<MethodData> methodData;
};

Plugin::Plugin(UniqueId id, ManifestPtr manifest) {
    PL_ASSERT(manifest->type == PackageType::Plugin && "Invalid package type for plugin ctor");
    PL_ASSERT(manifest->path.has_parent_path() && "Package path doesn't contain parent path");
    _impl->id = id;
    _impl->manifest = std::static_pointer_cast<PluginManifest>(std::move(manifest));
}
Plugin::~Plugin() = default;

Plugin::Plugin(const Plugin& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {}

Plugin::Plugin(Plugin&& other) noexcept = default;

Plugin& Plugin::operator=(const Plugin& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}
Plugin& Plugin::operator=(Plugin&& other) noexcept = default;

/*Result<void> Plugin::Load() {
    StateTransition scope(_impl, PluginState::Unloaded, PluginState::Loaded);

    auto result = _impl->module->LoadPlugin(*this);
    if (!result) {
        return plg::unexpected(result.error());
    }

    scope.Commit();
    return {};
}

void Plugin::Start() {
    StateTransition scope(_impl, PluginState::Loaded, PluginState::Started);

    _impl->module->StartPlugin(*this);

    scope.Commit();
}

void Plugin::Update(double deltaTime) {
    StateTransition scope(_impl, PluginState::Started, PluginState::Started);

    _impl->module->UpdatePlugin(*this, deltaTime);

    scope.Commit();
}

void Plugin::End() {
    StateTransition scope(_impl, PluginState::Started, PluginState::Ended);

    _impl->module->EndPlugin(*this);

    scope.Commit();
}

void Plugin::Unload() {
    StateTransition scope(_impl, PluginState::Ended, PluginState::Unloaded);

    scope.Commit();
}*/

static std::string emptyString;
static std::vector<std::string> emptyStrings;
static std::vector<Dependency> emptyDependencies;
static std::vector<Conflict> emptyConflicts;
static std::vector<Obsolete> emptyObsoletes;
static std::vector<Method> emptyMethods;

UniqueId Plugin::GetId() const noexcept { return _impl->id; }
PluginState Plugin::GetState() const noexcept { return _impl->state; }
std::shared_ptr<Module> Plugin::GetModule() const noexcept { return _impl->module; }
const std::string& Plugin::GetName() const noexcept { return _impl->manifest->name; }
PackageType Plugin::GetType() const noexcept { return _impl->manifest->type; }
const Version& Plugin::GetVersion() const noexcept { return _impl->manifest->version; }
const std::string& Plugin::GetDescription() const noexcept { return _impl->manifest->description ? *_impl->manifest->description : emptyString; }
const std::string& Plugin::GetAuthor() const noexcept { return _impl->manifest->author ? *_impl->manifest->author : emptyString; }
const std::string& Plugin::GetWebsite() const noexcept { return _impl->manifest->website ? *_impl->manifest->website : emptyString; }
const std::string& Plugin::GetLicense() const noexcept { return _impl->manifest->license ? *_impl->manifest->license : emptyString; }
const std::filesystem::path& Plugin::GetLocation() const noexcept { return _impl->manifest->path; }
const std::vector<std::string>& Plugin::GetPlatforms() const noexcept {
    return _impl->manifest->platforms ? *_impl->manifest->platforms : emptyStrings;
}
const std::vector<Dependency>& Plugin::GetDependencies() const noexcept {
    return _impl->manifest->dependencies ? *_impl->manifest->dependencies : emptyDependencies;
}
const std::vector<Conflict>& Plugin::GetConflicts() const noexcept {
    return _impl->manifest->conflicts ? *_impl->manifest->conflicts : emptyConflicts;
}
const std::vector<Obsolete>& Plugin::GetObsoletes() const noexcept {
    return _impl->manifest->obsoletes ? *_impl->manifest->obsoletes : emptyObsoletes;
}
const std::string& Plugin::GetLanguage() const noexcept { return _impl->manifest->language; }
const std::string& Plugin::GetEntry() const noexcept { return _impl->manifest->entry; }
const std::vector<Method>& Plugin::GetMethods() const { return _impl->manifest->methods ? *_impl->manifest->methods : emptyMethods; }
const std::vector<MethodData>& Plugin::GetMethodsData() const noexcept { return _impl->methodData; }
const std::filesystem::path& Plugin::GetBaseDir() const noexcept { return _impl->paths.base; }
const std::filesystem::path& Plugin::GetConfigsDir() const noexcept { return _impl->paths.configs; }
const std::filesystem::path& Plugin::GetDataDir() const noexcept { return _impl->paths.data; }
const std::filesystem::path& Plugin::GetLogsDir() const noexcept { return _impl->paths.logs; }
ILanguageModule* Plugin::GetLanguageModule() const noexcept { return _impl->languageModule; }
MemAddr Plugin::GetUserData() const noexcept { return _impl->userData; }
MethodTable Plugin::GetTable() const noexcept { return _impl->table; }

void Plugin::SetId(UniqueId id) noexcept { _impl->id = id; }
void Plugin::SetState(PluginState state) noexcept { _impl->state = state; }
void Plugin::SetModule(std::shared_ptr<Module> module) noexcept { _impl->module = std::move(module);}
void Plugin::SetName(std::string name) noexcept { _impl->manifest->name = std::move(name); }
void Plugin::SetType(PackageType type) noexcept { _impl->manifest->type = std::move(type); }
void Plugin::SetVersion(Version version) noexcept { _impl->manifest->version = std::move(version); }
void Plugin::SetDescription(std::string description) noexcept { 
	_impl->manifest->description = std::move(description);
}
void Plugin::SetAuthor(std::string author) noexcept { _impl->manifest->author = std::move(author); }
void Plugin::SetWebsite(std::string website) noexcept { _impl->manifest->website = std::move(website); }
void Plugin::SetLicense(std::string license) noexcept { _impl->manifest->license = std::move(license); }
void Plugin::SetLocation(std::filesystem::path location) noexcept { 
	_impl->manifest->path = std::move(location);
}
void Plugin::SetPlatforms(std::vector<std::string> platforms) noexcept {
	_impl->manifest->platforms = std::move(platforms);
}
void Plugin::SetDependencies(std::vector<Dependency> dependencies) noexcept {
	_impl->manifest->dependencies = std::move(dependencies);
}
void Plugin::SetConflicts(std::vector<Conflict> conflicts) noexcept {
	_impl->manifest->conflicts = std::move(conflicts);
}
void Plugin::SetObsoletes(std::vector<Obsolete> obsoletes) noexcept {
	_impl->manifest->obsoletes = std::move(obsoletes);
}
void Plugin::SetLanguage(std::string language) noexcept { _impl->manifest->language = std::move(language); }
void Plugin::SetEntry(std::string entry) noexcept { _impl->manifest->entry = std::move(entry); }
void Plugin::SetMethods(std::vector<Method> methods) noexcept {
    _impl->manifest->methods = std::move(methods);
}
void Plugin::SetMethodsData(std::vector<MethodData> methodsData) noexcept {
    _impl->methodData = std::move(methodsData);
}
void Plugin::SetBaseDir(std::filesystem::path base) noexcept { _impl->paths.base = std::move(base); }
void Plugin::SetConfigsDir(std::filesystem::path configs) noexcept { _impl->paths.configs = std::move(configs); }
void Plugin::SetDataDir(std::filesystem::path data) noexcept { _impl->paths.data = std::move(data); }
void Plugin::SetLogsDir(std::filesystem::path logs) noexcept { _impl->paths.logs = std::move(logs); }
void Plugin::SetLanguageModule(ILanguageModule* languageModule) noexcept { _impl->languageModule = languageModule; }
void Plugin::SetUserData(MemAddr userData) noexcept { _impl->userData = userData; }
void Plugin::SetTable(MethodTable table) noexcept { _impl->table = table; }
bool Plugin::operator==(const Plugin& other) const noexcept = default;
auto Plugin::operator<=>(const Plugin& other) const noexcept = default;
