#include "plugify/core/assembly.hpp"
#include "plugify/core/conflict.hpp"
#include "plugify/core/dependency.hpp"
#include "plugify/core/language_module.hpp"
#include "plugify/core/method.hpp"
#include "plugify/core/module.hpp"
#include "plugify/core/plugin.hpp"
#include "plugify/core/provider.hpp"

using namespace plugify;

// Internal state
struct Module::Impl {
    UniqueId id;
    ILanguageModule* languageModule{ nullptr };
    ModuleState state{ ModuleState::Unloaded };
    MethodTable table;
    std::shared_ptr<ModuleManifest> manifest;
    std::shared_ptr<IAssembly> assembly;
};

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

// Default empty containers
static std::string emptyString;
static std::filesystem::path emptyPath;
static std::vector<std::string> emptyStrings;
static std::vector<std::filesystem::path> emptyPaths;
static std::vector<Dependency> emptyDependencies;
static std::vector<Conflict> emptyConflicts;
static std::vector<Obsolete> emptyObsoletes;

// Getters
const UniqueId& Module::GetId() const noexcept { return _impl->id; }
ModuleState Module::GetState() const noexcept { return _impl->state; }
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
const std::vector<std::filesystem::path> Module::GetDirectories() const noexcept { return _impl->manifest->directories ? *_impl->manifest->directories : emptyPaths; }

const std::filesystem::path& Module::GetBaseDir() const noexcept { return _impl->paths.base; }
const std::filesystem::path& Module::GetConfigsDir() const noexcept { return _impl->paths.configs; }
const std::filesystem::path& Module::GetDataDir() const noexcept { return _impl->paths.data; }
const std::filesystem::path& Module::GetLogsDir() const noexcept { return _impl->paths.logs; }

// Setters
void Module::SetId(UniqueId id) noexcept { _impl->id = id; }
void Module::SetState(ModuleState state) noexcept { _impl->state = state; }
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
void Module::SetDirectories(std::vector<std::filesystem::path> directories) noexcept { _impl->manifest->directories = std::move(directories); }

void Module::SetBaseDir(std::filesystem::path base) noexcept { _impl->paths.base = std::move(base); }
void Module::SetConfigsDir(std::filesystem::path configs) noexcept { _impl->paths.configs = std::move(configs); }
void Module::SetDataDir(std::filesystem::path data) noexcept { _impl->paths.data = std::move(data); }
void Module::SetLogsDir(std::filesystem::path logs) noexcept { _impl->paths.logs = std::move(logs); }

// Comparison
bool Module::operator==(const Module& other) const noexcept = default;
auto Module::operator<=>(const Module& other) const noexcept = default;