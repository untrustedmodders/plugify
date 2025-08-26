#pragma once

#include "plugify/core/manifest.hpp"

namespace plugify {
    class Plugify;
	class Dependency;
	class Conflict;
	// Module Class
    class PLUGIFY_API Module {
        struct Impl;
	public:
	    Module(UniqueId id, ManifestPtr manifest);
	    ~Module();
	    Module(const Module& other);
	    Module(Module&& other) noexcept;
	    Module& operator=(const Module& other);
	    Module& operator=(Module&& other) noexcept;

    PLUGIFY_ACCESS:
        Result<void> Initialize();
        Result<void> Load(Plugify& plugify);
        void Update();
        void Unload();
	    void Terminate();

        //bool LoadPlugin(std::shared_ptr<Plugin> plugin) const;
        //void StartPlugin(std::shared_ptr<Plugin> plugin) const;
        //void UpdatePlugin(std::shared_ptr<Plugin> plugin, DateTime dt) const;
        //void EndPlugin(std::shared_ptr<Plugin> plugin) const;
        //void MethodExport(std::shared_ptr<Plugin> plugin) const;

    public:
	    // Getters
	    [[nodiscard]] const UniqueId& GetId() const noexcept;
	    [[nodiscard]] const std::string& GetName() const noexcept;
	    [[nodiscard]] PackageType GetType() const noexcept;
	    [[nodiscard]] const Version& GetVersion() const noexcept;
	    [[nodiscard]] const std::string& GetDescription() const noexcept;
	    [[nodiscard]] const std::string& GetAuthor() const noexcept;
	    [[nodiscard]] const std::string& GetWebsite() const noexcept;
	    [[nodiscard]] const std::string& GetLicense() const noexcept;
	    [[nodiscard]] const std::filesystem::path& GetLocation() const noexcept;
	    [[nodiscard]] const std::vector<std::string> GetPlatforms() const noexcept;
	    [[nodiscard]] const std::vector<Dependency> GetDependencies() const noexcept;
	    [[nodiscard]] const std::vector<Conflict> GetConflicts() const noexcept;
	    [[nodiscard]] const std::vector<Obsolete> GetObsoletes() const noexcept;

		// Getters
		[[nodiscard]] const std::string& GetLanguage() const noexcept;
		[[nodiscard]] const std::filesystem::path& GetRuntime() const noexcept;
		[[nodiscard]] const std::vector<std::string> GetDirectories() const noexcept;
		[[nodiscard]] bool GetForceLoad() const noexcept;

        // Getters
        const std::filesystem::path& GetBaseDir() const noexcept;
        const std::filesystem::path& GetConfigsDir() const noexcept;
        const std::filesystem::path& GetDataDir() const noexcept;
        const std::filesystem::path& GetLogsDir() const noexcept;

	    // Setters
	    void SetId(UniqueId id) noexcept;
	    void SetName(std::string name) noexcept;
	    void SetType(PackageType type) noexcept;
	    void SetVersion(Version version) noexcept;
	    void SetDescription(std::string description) noexcept;
	    void SetAuthor(std::string author) noexcept;
	    void SetWebsite(std::string website) noexcept;
	    void SetLicense(std::string license) noexcept;
	    void SetLocation(std::filesystem::path location) noexcept;
	    void SetPlatforms(std::vector<std::string> platforms) noexcept;
	    void SetDependencies(std::vector<Dependency> dependencies) noexcept;
	    void SetConflicts(std::vector<Conflict> conflicts) noexcept;
	    void SetObsoletes(std::vector<Obsolete> obsoletes) noexcept;

		// Setters (pass by value and move)
		void SetLanguage(std::string language) noexcept;
		void SetRuntime(std::filesystem::path runtimePath) noexcept;
		void SetDirectories(std::vector<std::string> directories) noexcept;
		void SetForceLoad(bool forceLoad) noexcept;

        // Setters
        void SetBaseDir(std::filesystem::path base) noexcept;
        void SetConfigsDir(std::filesystem::path configs) noexcept;
        void SetDataDir(std::filesystem::path data) noexcept;
        void SetLogsDir(std::filesystem::path logs) noexcept;

		[[nodiscard]] bool operator==(const Module& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Module& other) const noexcept;

		static inline constexpr std::string_view kFileExtension = "*.pmodule";
		static inline constexpr std::string_view kGetLanguageModuleFn = "GetLanguageModule";

	PLUGIFY_ACCESS:
	    std::unique_ptr<Impl> _impl;
	};
}