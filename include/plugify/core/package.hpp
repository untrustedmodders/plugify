#pragma once

#include <optional>

#include "plugify/core/manifest.hpp"
namespace plugify {
    // Package State Enum
    enum class PackageState {
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

        Starting,
        Started,

        Updating,
        Updated,
        /**/

        Ending,
        Ended,

        Terminated,
        Max
    };
    // Unified Package class
    class PLUGIFY_API Package {
        struct Impl;

    public:
        Package(UniqueId id, std::filesystem::path location);
        ~Package();

        // Delete copy/move for safety (can be implemented if needed)
        Package(const Package&) = delete;
        Package(Package&&) noexcept;
        Package& operator=(const Package&) = delete;
        Package& operator=(Package&&) noexcept;

        // --- Core Getters ---
        [[nodiscard]] UniqueId GetId() const noexcept;
        [[nodiscard]] PackageType GetType() const noexcept;
        [[nodiscard]] PackageState GetState() const noexcept;
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
        [[nodiscard]] const PackageManifest& GetManifest() const noexcept;

        // --- State & Error Management ---
        [[nodiscard]] const std::deque<std::string>& GetErrors() const noexcept;
        [[nodiscard]] const std::deque<std::string>& GetWarnings() const noexcept;
        [[nodiscard]] bool HasErrors() const noexcept;
        [[nodiscard]] bool IsLoaded() const noexcept;
        [[nodiscard]] bool IsPlugin() const noexcept { return GetType() == PackageType::Plugin; }
        [[nodiscard]] bool IsModule() const noexcept { return GetType() == PackageType::Module; }

        // --- Timing/Performance ---
        [[nodiscard]] Duration GetOperationTime(PackageState state) const;
        [[nodiscard]] Duration GetTotalTime() const;
        [[nodiscard]] std::string GetPerformanceReport() const;

        // --- State Management ---
        void StartOperation(PackageState newState);
        void EndOperation(PackageState newState);
        void SetState(PackageState state);

        // --- Error/Warning Management ---
        void AddError(std::string error);
        void AddWarning(std::string warning);
        void ClearErrors();
        void ClearWarnings();

        // --- Runtime Updates ---
        void SetUserData(MemAddr data);
        void SetMethodTable(MethodTable table);
        void SetLanguageModule(ILanguageModule* module);
        void SetManifest(PackageManifest manifest);

        // --- Plugin-specific setters ---
        void SetMethodsData(std::vector<MethodData> methodsData);

        // --- Module-specific setters ---
        void SetAssembly(std::shared_ptr<IAssembly> assembly);

        // --- Comparison ---
        [[nodiscard]] bool operator==(const Package& other) const noexcept;
        [[nodiscard]] auto operator<=>(const Package& other) const noexcept;

        // --- File extensions ---
        [[nodiscard]] static std::string_view GetFileExtension(PackageType type);
        [[nodiscard]] static PackageType GetPackageType(const std::filesystem::path& path);

        // --- Helpers ---
        [[nodiscard]] static bool IsValidTransition(PackageState from, PackageState to);
        [[nodiscard]] std::string ToString() const;
        bool CanLoad() const noexcept;
        bool CanStart() const noexcept;
        bool CanStop() const noexcept;
        void AddDependency(std::string dep);
        void Reset();

    private:
        std::unique_ptr<Impl> _impl;
    };
}