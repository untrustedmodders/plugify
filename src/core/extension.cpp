#include "plugify/assembly.hpp"
#include "plugify/extension.hpp"
#include "plugify/language_module.hpp"
#include "plugify/registrar.hpp"

using namespace plugify;

// Implementation structure
struct Extension::Impl {
    // Core identity
    UniqueId id{-1};
    ExtensionType type{ExtensionType::Unknown};
    ExtensionState state{ExtensionState::Unknown};

    // Runtime state
    MethodTable methodTable;
    MemAddr userData;
    ILanguageModule* languageModule{nullptr};
    Manifest manifest;
    std::filesystem::path location;
    std::string version;

    // Timing
    struct Timings {
        plg::flat_map<ExtensionState, Duration> timepoints;
        TimePoint lastOperationStart;

        Duration GetTotalTime() const {
            Duration total{};
            for (const auto& [_, t] : timepoints) {
                total += t;
            }
            return total;
        }

        std::string ToString() const {
            std::string merged;
            merged.reserve(256);
            auto it = std::back_inserter(merged);
            for (const auto& [s, t] : timepoints) {
                std::format_to(it, "  {} {},\n", plg::enum_to_string(s), t);
            }
            std::format_to(it, "  - Total {}", GetTotalTime());
            return merged;
        }
    } timings;

    // Error tracking
    std::unique_ptr<Registrar> registrar;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    // Plugin-specific data (only populated for plugins)
    struct PluginData {
        std::vector<MethodData> methodData;
    } pluginData;

    // Module-specific data (only populated for modules)
    struct ModuleData {
        std::shared_ptr<IAssembly> assembly;
    } moduleData;

    void Cache() {
        id.SetName(manifest.name);
        version = manifest.version.to_string();
        location = location.parent_path();
        if (type == ExtensionType::Module && !manifest.runtime) {
            // Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.
            manifest.runtime = location / "bin" / std::format(PLUGIFY_LIBRARY_PREFIX "{}" PLUGIFY_LIBRARY_SUFFIX, manifest.name);
        }
        registrar = std::make_unique<Registrar>(id, Registrar::DebugInfo{
            .name = manifest.name,
            .type = type,
            .version = version,
            .location = location,
        });
    }
};

// Static empty defaults for returning const references to empty containers
static const std::string kEmptyString;
static const std::filesystem::path kEmptyPath;
static const std::vector<std::string> kEmptyStrings;
static const std::vector<std::filesystem::path> kEmptyPaths;
static const std::vector<Dependency> kEmptyDependencies;
static const std::vector<Conflict> kEmptyConflicts;
static const std::vector<Obsolete> kEmptyObsoletes;
static const std::vector<Method> kEmptyMethods;
static const std::vector<MethodData> kEmptyMethodData;
//static const std::deque<std::string> kEmptyErrors;

// ============================================================================
// Extension::Impl Definition
// ============================================================================

// ============================================================================
// Constructor & Destructor
// ============================================================================

Extension::Extension(UniqueId id, std::filesystem::path location)
    : _impl(std::make_unique<Impl>()) {
    _impl->id = id;
    _impl->state = ExtensionState::Discovered;
    _impl->type = GetExtensionType(location);
    _impl->location = std::move(location);
}

Extension::~Extension() = default;

Extension::Extension(Extension&& other) noexcept = default;

Extension& Extension::operator=(Extension&& other) noexcept = default;

// ============================================================================
// Core Getters
// ============================================================================

UniqueId Extension::GetId() const noexcept {
    return _impl->id;
}

ExtensionType Extension::GetType() const noexcept {
    return _impl->type;
}

ExtensionState Extension::GetState() const noexcept {
    return _impl->state;
}

const std::string& Extension::GetName() const noexcept {
    return _impl->manifest.name;
}

const Version& Extension::GetVersion() const noexcept {
    return _impl->manifest.version;
}

const std::string& Extension::GetLanguage() const noexcept {
    return _impl->manifest.language;
}

const std::filesystem::path& Extension::GetLocation() const noexcept {
    return _impl->location;
}

// ============================================================================
// Optional Info Getters
// ============================================================================

const std::string& Extension::GetDescription() const noexcept {
    return _impl->manifest.description ? *_impl->manifest.description : kEmptyString;
}

const std::string& Extension::GetAuthor() const noexcept {
    return _impl->manifest.author ? *_impl->manifest.author : kEmptyString;
}

const std::string& Extension::GetWebsite() const noexcept {
    return _impl->manifest.website ? *_impl->manifest.website : kEmptyString;
}

const std::string& Extension::GetLicense() const noexcept {
    return _impl->manifest.license ? *_impl->manifest.license : kEmptyString;
}

// ============================================================================
// Dependencies/Conflicts Getters
// ============================================================================

const std::vector<std::string>& Extension::GetPlatforms() const noexcept {
    return _impl->manifest.platforms ? *_impl->manifest.platforms : kEmptyStrings;
}

const std::vector<Dependency>& Extension::GetDependencies() const noexcept {
    return _impl->manifest.dependencies ? *_impl->manifest.dependencies : kEmptyDependencies;
}

const std::vector<Conflict>& Extension::GetConflicts() const noexcept {
    return _impl->manifest.conflicts ? *_impl->manifest.conflicts : kEmptyConflicts;
}

const std::vector<Obsolete>& Extension::GetObsoletes() const noexcept {
    return _impl->manifest.obsoletes ? *_impl->manifest.obsoletes : kEmptyObsoletes;
}

// ============================================================================
// Plugin-specific Getters
// ============================================================================

const std::string& Extension::GetEntry() const noexcept {
    if (_impl->type == ExtensionType::Plugin && _impl->manifest.entry) {
        return *_impl->manifest.entry;
    }
    return kEmptyString;
}

const std::vector<Method>& Extension::GetMethods() const noexcept {
    if (_impl->type == ExtensionType::Plugin && _impl->manifest.methods) {
        return *_impl->manifest.methods;
    }
    return kEmptyMethods;
}

const std::vector<MethodData>& Extension::GetMethodsData() const noexcept {
    if (_impl->type == ExtensionType::Plugin) {
        return _impl->pluginData.methodData;
    }
    return kEmptyMethodData;
}

// ============================================================================
// Module-specific Getters
// ============================================================================

const std::filesystem::path& Extension::GetRuntime() const noexcept {
    if (_impl->type == ExtensionType::Module && _impl->manifest.runtime) {
        return *_impl->manifest.runtime;
    }
    return kEmptyPath;
}

const std::vector<std::filesystem::path>& Extension::GetDirectories() const noexcept {
    if (_impl->type == ExtensionType::Module && _impl->manifest.directories) {
        return *_impl->manifest.directories;
    }
    return kEmptyPaths;
}

std::shared_ptr<IAssembly> Extension::GetAssembly() const noexcept {
    if (_impl->type == ExtensionType::Module) {
        return _impl->moduleData.assembly;
    }
    return nullptr;
}

// ============================================================================
// Shared Runtime Getters
// ============================================================================

MemAddr Extension::GetUserData() const noexcept {
    return _impl->userData;
}

MethodTable Extension::GetMethodTable() const noexcept {
    return _impl->methodTable;
}

ILanguageModule* Extension::GetLanguageModule() const noexcept {
    return _impl->languageModule;
}

const Manifest& Extension::GetManifest() const noexcept {
    return _impl->manifest;
}

// ============================================================================
// State & Error Management Getters
// ============================================================================

const std::vector<std::string>& Extension::GetErrors() const noexcept {
    return _impl->errors;
}

const std::vector<std::string>& Extension::GetWarnings() const noexcept {
    return _impl->warnings;
}

bool Extension::HasErrors() const noexcept {
    return !_impl->errors.empty();
}

bool Extension::HasWarnings() const noexcept {
    return !_impl->warnings.empty();
}

/*bool Extension::IsLoaded() const noexcept {
    switch (_impl->state) {
        case ExtensionState::Loaded:
        case ExtensionState::Started:
            return true;
        default:
            return false;
    }
}*/

// ============================================================================
// Timing/Performance Getters
// ============================================================================

Duration Extension::GetOperationTime(ExtensionState state) const {
    auto it = _impl->timings.timepoints.find(state);
    if (it != _impl->timings.timepoints.end()) {
        return it->second;
    }
    return Duration{};
}

Duration Extension::GetTotalTime() const {
    return _impl->timings.GetTotalTime();
}

std::string Extension::GetPerformanceReport() const {
    return std::format("{}:\n{}", GetName(), _impl->timings.ToString());
}

// ============================================================================
// State Management
// ============================================================================

void Extension::StartOperation(ExtensionState newState) {
    _impl->timings.lastOperationStart = Clock::now();
    SetState(newState);
}

void Extension::EndOperation(ExtensionState newState) {
    auto duration = std::chrono::duration_cast<Duration>(
        Clock::now() - _impl->timings.lastOperationStart
    );

    // Store the time for the operation that just ended (current state)
    _impl->timings.timepoints[_impl->state] = duration;

    // Transition to the new state
    SetState(newState);
}

void Extension::SetState(ExtensionState state) {
    assert(IsValidTransition(_impl->state, state) && "Invalid state transition");
    _impl->state = state;
}

// ============================================================================
// Error/Warning Management
// ============================================================================

void Extension::AddError(std::string error) {
    _impl->errors.push_back(std::move(error));
}

void Extension::AddWarning(std::string warning) {
    _impl->warnings.push_back(std::move(warning));
}

void Extension::ClearErrors() {
    _impl->errors.clear();
}

void Extension::ClearWarnings() {
    _impl->warnings.clear();
}

// ============================================================================
// Runtime Updates
// ============================================================================

void Extension::SetUserData(MemAddr data) {
    _impl->userData = data;
}

void Extension::SetMethodTable(MethodTable table) {
    _impl->methodTable = table;
}

void Extension::SetLanguageModule(ILanguageModule* module) {
    _impl->languageModule = module;
}

void Extension::SetManifest(Manifest manifest) {
    _impl->manifest = std::move(manifest);
    _impl->Cache();
}

// ============================================================================
// Plugin-specific Setters
// ============================================================================

void Extension::SetMethodsData(std::vector<MethodData> methodsData) {
    if (_impl->type == ExtensionType::Plugin) {
        _impl->pluginData.methodData = std::move(methodsData);
    }
}

// ============================================================================
// Module-specific Setters
// ============================================================================

void Extension::SetAssembly(std::shared_ptr<IAssembly> assembly) {
    if (_impl->type == ExtensionType::Module) {
        _impl->moduleData.assembly = std::move(assembly);
    }
}

// ============================================================================
// Comparison Operators
// ============================================================================

bool Extension::operator==(const Extension& other) const noexcept {
    return _impl->id == other._impl->id;
}

auto Extension::operator<=>(const Extension& other) const noexcept {
    return _impl->id <=> other._impl->id;
}

// ============================================================================
// Static Helper Methods
// ============================================================================

std::string_view Extension::GetFileExtension(ExtensionType type) {
    switch(type) {
        case ExtensionType::Plugin: return ".pplugin";
        case ExtensionType::Module: return ".pmodule";
        default: return "";
    }
}

ExtensionType Extension::GetExtensionType(const std::filesystem::path& path) {
    static std::unordered_map<std::string, ExtensionType, plg::case_insensitive_hash, plg::case_insensitive_equal> manifests = {
        { ".pplugin", ExtensionType::Plugin },
        { ".pmodule", ExtensionType::Module }
    };
    return manifests[path.extension().string()];
}

// ============================================================================
// Additional Helper Methods (Optional)
// ============================================================================

// Helper to validate state transitions (optional implementation)
bool Extension::IsValidTransition(ExtensionState from, ExtensionState to) {
    // Define valid state transitions
    // This is just an example - adjust based on your actual state machine
    switch (from) {
        case ExtensionState::Unknown:
            return to == ExtensionState::Discovered;

        case ExtensionState::Discovered:
            return to == ExtensionState::Parsing ||
                   to == ExtensionState::Failed;

        case ExtensionState::Parsing:
            return to == ExtensionState::Parsed ||
                   to == ExtensionState::Corrupted;

        case ExtensionState::Parsed:
            return to == ExtensionState::Resolving ||
                   to == ExtensionState::Failed ||
                   to == ExtensionState::Disabled;

        case ExtensionState::Resolving:
            return to == ExtensionState::Resolved ||
                   to == ExtensionState::Unresolved ||
                   to == ExtensionState::Disabled;

        case ExtensionState::Resolved:
            return to == ExtensionState::Loading ||
                   to == ExtensionState::Skipped ||
                   to == ExtensionState::Failed;

        case ExtensionState::Loading:
            return to == ExtensionState::Loaded ||
                   to == ExtensionState::Failed;

        case ExtensionState::Loaded:
            return to == ExtensionState::Exporting ||
                   to == ExtensionState::Skipped ||
                   to == ExtensionState::Failed;

        case ExtensionState::Exporting:
            return to == ExtensionState::Exported ||
                   to == ExtensionState::Skipped ||
                   to == ExtensionState::Failed;

        case ExtensionState::Exported:
            return to == ExtensionState::Starting ||
                   to == ExtensionState::Skipped ||
                   to == ExtensionState::Failed;

        case ExtensionState::Starting:
            return to == ExtensionState::Started ||
                   to == ExtensionState::Skipped ||
                   to == ExtensionState::Failed;

        case ExtensionState::Started:
            return to == ExtensionState::Running ||
                   to == ExtensionState::Failed;

        case ExtensionState::Running:
            return to == ExtensionState::Ending ||
                   to == ExtensionState::Failed;

        case ExtensionState::Ending:
            return to == ExtensionState::Ended ||
                   to == ExtensionState::Failed;

        case ExtensionState::Ended:
            return to == ExtensionState::Terminated;

        case ExtensionState::Failed:
        case ExtensionState::Terminated:
        case ExtensionState::Corrupted:
        case ExtensionState::Unresolved:
        case ExtensionState::Disabled:
        case ExtensionState::Skipped:
            return false; // Terminal states

        default:
            return false;
    }
}

// Debug/logging helper
std::string Extension::ToString() const {
    return std::format("Extension[id={}, name={}, type={}, state={}, errors={}, warnings={}]",
                      _impl->id,
                      GetName(),
                      plg::enum_to_string(_impl->type),
                      plg::enum_to_string(_impl->state),
                      _impl->errors.size(),
                      _impl->warnings.size());
}

const std::string& Extension::GetVersionString() const noexcept {
    return _impl->version;
}
// Helper to check if extension can be loaded
/*bool Extension::CanLoad() const noexcept {
    return (_impl->state == ExtensionState::Loading) && !HasErrors();
}

// Helper to check if extension can be started
bool Extension::CanStart() const noexcept {
    return _impl->state == ExtensionState::Loaded && !HasErrors();
}

// Helper to check if extension can be updated
bool Extension::CanUpdate() const noexcept {
    return _impl->state == ExtensionState::Started && !HasErrors();
}

// Helper to check if extension can be stopped
bool Extension::CanStop() const noexcept {
    return _impl->state == ExtensionState::Started && !HasErrors();
}*/

// Add dependency helper (for runtime dependency injection)
void Extension::AddDependency(std::string_view dep) {
    if (!_impl->manifest.dependencies) {
        _impl->manifest.dependencies = std::vector<Dependency>{};
    }
    _impl->manifest.dependencies->emplace_back().SetName(std::string(dep));
}

// Reset extension to initial state (useful for reload scenarios)
void Extension::Reset() {
    _impl->state = ExtensionState::Unknown;
    _impl->registrar.reset();
    _impl->errors.clear();
    _impl->warnings.clear();
    _impl->timings.timepoints.clear();
    _impl->timings.lastOperationStart = {};
    _impl->languageModule = nullptr;
    _impl->methodTable = {};
    _impl->userData = nullptr;
    _impl->manifest = {};
    _impl->location.clear();
    _impl->version.clear();

    if (_impl->type == ExtensionType::Plugin) {
        _impl->pluginData.methodData.clear();
    } else if (_impl->type == ExtensionType::Module) {
        _impl->moduleData.assembly.reset();
    }
}