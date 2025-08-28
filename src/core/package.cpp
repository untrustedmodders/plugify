#include "plugify/core/assembly.hpp"
#include "plugify/core/language_module.hpp"
#include "plugify/core/package.hpp"

using namespace plugify;

// Implementation structure
struct Package::Impl {
    // Core identity
    UniqueId id{-1};
    PackageType type{PackageType::Unknown};
    PackageState state{PackageState::Unknown};

    // Runtime state
    MethodTable methodTable;
    MemAddr userData;
    ILanguageModule* languageModule{nullptr};
    PackageManifest manifest;
    std::filesystem::path location;

    // Timing
    struct Timings {
        plg::flat_map<PackageState, Duration> timepoints;
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
            for (const auto& [state, timepoint] : timepoints) {
                std::format_to(std::back_inserter(merged), "{} {}ms, ",
                              plg::enum_to_string(state), timepoint.count());
            }
            std::format_to(std::back_inserter(merged), "Total {}ms", GetTotalTime().count());
            return merged;
        }
    } timings;

    // Error tracking
    std::deque<std::string> errors;
    std::deque<std::string> warnings;

    // Plugin-specific data (only populated for plugins)
    struct PluginData {
        std::vector<MethodData> methodData;
    } pluginData;

    // Module-specific data (only populated for modules)
    struct ModuleData {
        std::shared_ptr<IAssembly> assembly;
    } moduleData;
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
// Package::Impl Definition
// ============================================================================

// ============================================================================
// Constructor & Destructor
// ============================================================================

Package::Package(UniqueId id, std::filesystem::path location)
    : _impl(std::make_unique<Impl>()) {

    /*if (!manifest.runtime || manifest.runtime->empty()) {
        // Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.
        manifest.runtime = manifest.runtime.value_or(path / "bin" / std::format(PLUGIFY_LIBRARY_PREFIX "{}" PLUGIFY_LIBRARY_SUFFIX, manifest.name));
    }*/
    _impl->id = id;
    _impl->state = PackageState::Discovered;
    _impl->type = GetPackageType(location);
    _impl->location = std::move(location);
    PL_ASSERT(_impl->type != PackageType::Unknown);
}

Package::~Package() = default;

Package::Package(Package&& other) noexcept = default;

Package& Package::operator=(Package&& other) noexcept = default;

// ============================================================================
// Core Getters
// ============================================================================

UniqueId Package::GetId() const noexcept {
    return _impl->id;
}

PackageType Package::GetType() const noexcept {
    return _impl->type;
}

PackageState Package::GetState() const noexcept {
    return _impl->state;
}

const std::string& Package::GetName() const noexcept {
    return _impl->manifest.name;
}

const Version& Package::GetVersion() const noexcept {
    return _impl->manifest.version;
}

const std::string& Package::GetLanguage() const noexcept {
    return _impl->manifest.language;
}

const std::filesystem::path& Package::GetLocation() const noexcept {
    return _impl->location;
}

// ============================================================================
// Optional Info Getters
// ============================================================================

const std::string& Package::GetDescription() const noexcept {
    return _impl->manifest.description ? *_impl->manifest.description : kEmptyString;
}

const std::string& Package::GetAuthor() const noexcept {
    return _impl->manifest.author ? *_impl->manifest.author : kEmptyString;
}

const std::string& Package::GetWebsite() const noexcept {
    return _impl->manifest.website ? *_impl->manifest.website : kEmptyString;
}

const std::string& Package::GetLicense() const noexcept {
    return _impl->manifest.license ? *_impl->manifest.license : kEmptyString;
}


// ============================================================================
// Dependencies/Conflicts Getters
// ============================================================================

const std::vector<std::string>& Package::GetPlatforms() const noexcept {
    return _impl->manifest.platforms ? *_impl->manifest.platforms : kEmptyStrings;
}

const std::vector<Dependency>& Package::GetDependencies() const noexcept {
    return _impl->manifest.dependencies ? *_impl->manifest.dependencies : kEmptyDependencies;
}

const std::vector<Conflict>& Package::GetConflicts() const noexcept {
    return _impl->manifest.conflicts ? *_impl->manifest.conflicts : kEmptyConflicts;
}

const std::vector<Obsolete>& Package::GetObsoletes() const noexcept {
    return _impl->manifest.obsoletes ? *_impl->manifest.obsoletes : kEmptyObsoletes;
}

// ============================================================================
// Plugin-specific Getters
// ============================================================================

const std::string& Package::GetEntry() const noexcept {
    if (_impl->type == PackageType::Plugin && _impl->manifest.entry) {
        return *_impl->manifest.entry;
    }
    return kEmptyString;
}

const std::vector<Method>& Package::GetMethods() const noexcept {
    if (_impl->type == PackageType::Plugin && _impl->manifest.methods) {
        return *_impl->manifest.methods;
    }
    return kEmptyMethods;
}

const std::vector<MethodData>& Package::GetMethodsData() const noexcept {
    if (_impl->type == PackageType::Plugin) {
        return _impl->pluginData.methodData;
    }
    return kEmptyMethodData;
}

// ============================================================================
// Module-specific Getters
// ============================================================================

const std::filesystem::path& Package::GetRuntime() const noexcept {
    if (_impl->type == PackageType::Module && _impl->manifest.runtime) {
        return *_impl->manifest.runtime;
    }
    return kEmptyPath;
}

const std::vector<std::filesystem::path>& Package::GetDirectories() const noexcept {
    if (_impl->type == PackageType::Module && _impl->manifest.directories) {
        return *_impl->manifest.directories;
    }
    return kEmptyPaths;
}

std::shared_ptr<IAssembly> Package::GetAssembly() const noexcept {
    if (_impl->type == PackageType::Module) {
        return _impl->moduleData.assembly;
    }
    return nullptr;
}

// ============================================================================
// Shared Runtime Getters
// ============================================================================

MemAddr Package::GetUserData() const noexcept {
    return _impl->userData;
}

MethodTable Package::GetMethodTable() const noexcept {
    return _impl->methodTable;
}

ILanguageModule* Package::GetLanguageModule() const noexcept {
    return _impl->languageModule;
}

const PackageManifest& Package::GetManifest() const noexcept {
    return _impl->manifest;
}

// ============================================================================
// State & Error Management Getters
// ============================================================================

const std::deque<std::string>& Package::GetErrors() const noexcept {
    return _impl->errors;
}

const std::deque<std::string>& Package::GetWarnings() const noexcept {
    return _impl->warnings;
}

bool Package::HasErrors() const noexcept {
    return !_impl->errors.empty();
}

bool Package::IsLoaded() const noexcept {
    switch (_impl->state) {
        case PackageState::Loaded:
        case PackageState::Started:
        case PackageState::Updated:
            return true;
        default:
            return false;
    }
}

// ============================================================================
// Timing/Performance Getters
// ============================================================================

Duration Package::GetOperationTime(PackageState state) const {
    auto it = _impl->timings.timepoints.find(state);
    if (it != _impl->timings.timepoints.end()) {
        return it->second;
    }
    return Duration{};
}

Duration Package::GetTotalTime() const {
    return _impl->timings.GetTotalTime();
}

std::string Package::GetPerformanceReport() const {
    return std::format("{}: {}", GetName(), _impl->timings.ToString());
}

// ============================================================================
// State Management
// ============================================================================

void Package::StartOperation(PackageState newState) {
    _impl->timings.lastOperationStart = Clock::now();
    SetState(newState);
}

void Package::EndOperation(PackageState newState) {
    auto duration = std::chrono::duration_cast<Duration>(
        Clock::now() - _impl->timings.lastOperationStart
    );

    // Store the time for the operation that just ended (current state)
    _impl->timings.timepoints[_impl->state] = duration;

    // Transition to the new state
    SetState(newState);
}

void Package::SetState(PackageState state) {
    PL_ASSERT(IsValidTransition(_impl->state, state) && "Invalid state transition");
    _impl->state = state;
}

// ============================================================================
// Error/Warning Management
// ============================================================================

void Package::AddError(std::string error) {
    _impl->errors.emplace_back(std::move(error));
}

void Package::AddWarning(std::string warning) {
    _impl->warnings.emplace_back(std::move(warning));
}

void Package::ClearErrors() {
    _impl->errors.clear();
}

void Package::ClearWarnings() {
    _impl->warnings.clear();
}

// ============================================================================
// Runtime Updates
// ============================================================================

void Package::SetUserData(MemAddr data) {
    _impl->userData = data;
}

void Package::SetMethodTable(MethodTable table) {
    _impl->methodTable = table;
}

void Package::SetLanguageModule(ILanguageModule* module) {
    _impl->languageModule = module;
}

void Package::SetManifest(PackageManifest manifest) {
    _impl->manifest = std::move(manifest);
    _impl->location = _impl->location.parent_path();
}

// ============================================================================
// Plugin-specific Setters
// ============================================================================

void Package::SetMethodsData(std::vector<MethodData> methodsData) {
    if (_impl->type == PackageType::Plugin) {
        _impl->pluginData.methodData = std::move(methodsData);
    }
}

// ============================================================================
// Module-specific Setters
// ============================================================================

void Package::SetAssembly(std::shared_ptr<IAssembly> assembly) {
    if (_impl->type == PackageType::Module) {
        _impl->moduleData.assembly = std::move(assembly);
    }
}

// ============================================================================
// Comparison Operators
// ============================================================================

bool Package::operator==(const Package& other) const noexcept {
    return _impl->id == other._impl->id;
}

auto Package::operator<=>(const Package& other) const noexcept {
    return _impl->id <=> other._impl->id;
}

// ============================================================================
// Static Helper Methods
// ============================================================================

std::string_view Package::GetFileExtension(PackageType type) {
    switch(type) {
        case PackageType::Plugin: return ".pplugin";
        case PackageType::Module: return ".pmodule";
        default: return "";
    }
}

PackageType Package::GetPackageType(const std::filesystem::path& path) {
    static std::unordered_map<std::string, PackageType, plg::case_insensitive_hash, plg::case_insensitive_equal> manifests = {
        { ".pplugin", PackageType::Plugin },
        { ".pmodule", PackageType::Module }
    };
    return manifests[path.extension().string()];
}

// ============================================================================
// Additional Helper Methods (Optional)
// ============================================================================

// Helper to validate state transitions (optional implementation)
bool Package::IsValidTransition(PackageState from, PackageState to) {
    // Define valid state transitions
    // This is just an example - adjust based on your actual state machine
    switch (from) {
        case PackageState::Unknown:
            return to == PackageState::Discovered;

        case PackageState::Discovered:
            return to == PackageState::Parsing ||
                   to == PackageState::Failed ||
                   to == PackageState::Disabled;

        case PackageState::Parsing:
            return to == PackageState::Parsed ||
                   to == PackageState::Corrupted;

        case PackageState::Parsed:
            return to == PackageState::Resolving ||
                   to == PackageState::Failed;

        case PackageState::Resolving:
            return to == PackageState::Resolved ||
                   to == PackageState::Unresolved;

        case PackageState::Resolved:
            return to == PackageState::Loading ||
                   to == PackageState::Skipped ||
                   to == PackageState::Failed;

        case PackageState::Loading:
            return to == PackageState::Loaded ||
                   to == PackageState::Failed;

        case PackageState::Loaded:
            return to == PackageState::Starting ||
                   to == PackageState::Ending ||
                   to == PackageState::Failed;

        case PackageState::Starting:
            return to == PackageState::Started ||
                   to == PackageState::Failed;

        case PackageState::Started:
            return to == PackageState::Updating ||
                   to == PackageState::Ending ||
                   to == PackageState::Failed;

        case PackageState::Updating:
            return to == PackageState::Updated ||
                   to == PackageState::Failed;

        case PackageState::Updated:
            return to == PackageState::Updating ||
                   to == PackageState::Ending ||
                   to == PackageState::Failed;

        case PackageState::Ending:
            return to == PackageState::Ended ||
                   to == PackageState::Failed;

        case PackageState::Ended:
            return to == PackageState::Terminated;

        case PackageState::Failed:
        case PackageState::Terminated:
        case PackageState::Corrupted:
        case PackageState::Unresolved:
        case PackageState::Disabled:
        case PackageState::Skipped:
            return false; // Terminal states

        default:
            return false;
    }
}

// Debug/logging helper
std::string Package::ToString() const {
    return std::format("Package[id={}, name={}, type={}, state={}, errors={}, warnings={}]",
                      _impl->id,
                      GetName(),
                      plg::enum_to_string(_impl->type),
                      plg::enum_to_string(_impl->state),
                      _impl->errors.size(),
                      _impl->warnings.size());
}

// Helper to check if package can be loaded
bool Package::CanLoad() const noexcept {
    return (_impl->state == PackageState::Resolved ||
            _impl->state == PackageState::Ended) &&
           !HasErrors();
}

// Helper to check if package can be started
bool Package::CanStart() const noexcept {
    return _impl->state == PackageState::Loaded && !HasErrors();
}

// Helper to check if package can be stopped
bool Package::CanStop() const noexcept {
    return _impl->state == PackageState::Started ||
           _impl->state == PackageState::Updated;
}

// Add dependency helper (for runtime dependency injection)
void Package::AddDependency(std::string dep) {
    if (!_impl->manifest.dependencies) {
        _impl->manifest.dependencies = std::vector<Dependency>{};
    }
    _impl->manifest.dependencies->emplace_back().SetName(std::move(dep));
}

// Reset package to initial state (useful for reload scenarios)
void Package::Reset() {
    _impl->state = PackageState::Unknown;
    _impl->languageModule = nullptr;
    _impl->errors.clear();
    _impl->warnings.clear();
    _impl->timings.timepoints.clear();
    _impl->methodTable = MethodTable{};
    _impl->userData = nullptr;

    if (_impl->type == PackageType::Plugin) {
        _impl->pluginData.methodData.clear();
    } else if (_impl->type == PackageType::Module) {
        _impl->moduleData.assembly.reset();
    }
}