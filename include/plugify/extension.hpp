#pragma once

#include <string>
#include <deque>
#include <vector>
#include <memory>
#include <filesystem>

#include "plugify/manifest.hpp"

namespace plugify {
    class IAssembly;
    class ILanguageModule;

    // Extension State Enum
    enum class ExtensionState {
        Unknown,
        Discovered,

        Parsing,
        Parsed,
        Corrupted,

        Resolving,
        Resolved,
        Unresolved,

        Disabled,
        Skipped,
        Failed,

        //Initializing,
        //Initialized,

        Loading,
        Loaded,

        Exporting,
        Exported,

        Starting,
        Started,

        Running,

        Ending,
        Ended,

        Terminating,
        Terminated,

        Max
    };
    // Unified Extension class
    class PLUGIFY_API Extension {
    public:
        Extension(UniqueId id, std::filesystem::path location);
        ~Extension();

        // Delete copy/move for safety (can be implemented if needed)
        Extension(const Extension&) = delete;
        Extension(Extension&&) noexcept;
        Extension& operator=(const Extension&) = delete;
        Extension& operator=(Extension&&) noexcept;

        // --- Core Getters ---
        [[nodiscard]] UniqueId GetId() const noexcept;
        [[nodiscard]] ExtensionType GetType() const noexcept;
        [[nodiscard]] ExtensionState GetState() const noexcept;
        [[nodiscard]] const std::string& GetName() const noexcept;
        [[nodiscard]] const Version& GetVersion() const noexcept;
        [[nodiscard]] const std::string& GetLanguage() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetLocation() const noexcept;

        // --- Optional Info Getters ---
        [[nodiscard]] const std::string& GetDescription() const noexcept;
        [[nodiscard]] const std::string& GetAuthor() const noexcept;
        [[nodiscard]] const std::string& GetWebsite() const noexcept;
        [[nodiscard]] const std::string& GetLicense() const noexcept;

        // --- Dependencies/Conflicts ---
        [[nodiscard]] const std::vector<std::string>& GetPlatforms() const noexcept;
        [[nodiscard]] const std::vector<Dependency>& GetDependencies() const noexcept;
        [[nodiscard]] const std::vector<Conflict>& GetConflicts() const noexcept;
        [[nodiscard]] const std::vector<Obsolete>& GetObsoletes() const noexcept;

        // --- Plugin-specific (returns empty/null for modules) ---
        [[nodiscard]] const std::string& GetEntry() const noexcept;
        [[nodiscard]] const std::vector<Method>& GetMethods() const noexcept;
        [[nodiscard]] const std::vector<MethodData>& GetMethodsData() const noexcept;

        // --- Module-specific (returns empty/null for plugins) ---
        [[nodiscard]] const std::filesystem::path& GetRuntime() const noexcept;
        [[nodiscard]] const std::vector<std::filesystem::path>& GetDirectories() const noexcept;
        [[nodiscard]] std::shared_ptr<IAssembly> GetAssembly() const noexcept;

        // --- Shared Runtime ---
        [[nodiscard]] MemAddr GetUserData() const noexcept;
        [[nodiscard]] MethodTable GetMethodTable() const noexcept;
        [[nodiscard]] ILanguageModule* GetLanguageModule() const noexcept;
        [[nodiscard]] const Manifest& GetManifest() const noexcept;

        // --- State & Error Management ---
        [[nodiscard]] const std::vector<std::string>& GetErrors() const noexcept;
        [[nodiscard]] const std::vector<std::string>& GetWarnings() const noexcept;
        [[nodiscard]] bool HasErrors() const noexcept;
        [[nodiscard]] bool HasWarnings() const noexcept;
        //[[nodiscard]] bool IsLoaded() const noexcept;
        [[nodiscard]] bool IsPlugin() const noexcept { return GetType() == ExtensionType::Plugin; }
        [[nodiscard]] bool IsModule() const noexcept { return GetType() == ExtensionType::Module; }

        // --- Timing/Performance ---
        [[nodiscard]] std::chrono::milliseconds GetOperationTime(ExtensionState state) const;
        [[nodiscard]] std::chrono::milliseconds GetTotalTime() const;
        [[nodiscard]] std::string GetPerformanceReport() const;

        // --- State Management ---
        void StartOperation(ExtensionState newState);
        void EndOperation(ExtensionState newState);
        void SetState(ExtensionState state);

        // --- Error/Warning Management ---
        void AddError(std::string error);
        void AddWarning(std::string warning);
        void ClearErrors();
        void ClearWarnings();

        // --- Runtime Updates ---
        void SetUserData(MemAddr data);
        void SetMethodTable(MethodTable table);
        void SetLanguageModule(ILanguageModule* module);
        void SetManifest(Manifest manifest);

        // --- Plugin-specific setters ---
        void SetMethodsData(std::vector<MethodData> methodsData);

        // --- Module-specific setters ---
        void SetAssembly(std::shared_ptr<IAssembly> assembly);

        // --- Comparison ---
        [[nodiscard]] bool operator==(const Extension& other) const noexcept;
        [[nodiscard]] auto operator<=>(const Extension& other) const noexcept;

        // --- File extensions ---
        [[nodiscard]] static plg::path_view GetFileExtension(ExtensionType type);
        [[nodiscard]] static ExtensionType GetExtensionType(const std::filesystem::path& path);

        // --- Helpers ---
        [[nodiscard]] static bool IsValidTransition(ExtensionState from, ExtensionState to);
        [[nodiscard]] std::string ToString() const;
        [[nodiscard]] const std::string& GetVersionString() const noexcept;
        //bool CanLoad() const noexcept;
        //bool CanStart() const noexcept;
        //bool CanUpdate() const noexcept;
        //bool CanStop() const noexcept;
        void AddDependency(std::string dep);
        void Reset();

    private:
        struct Impl;
        PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
    };
}