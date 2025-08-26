#pragma once

#include "plugify/core/manifest.hpp"

namespace plugify {
    class Plugify;
    class Dependency;
    class Conflict;
    class Module;
	// Plugin Class
	class PLUGIFY_API Plugin {
	    struct Impl;
	public:
	    Plugin(UniqueId id, ManifestPtr manifest);
	    ~Plugin();
	    Plugin(const Plugin& other);
	    Plugin(Plugin&& other) noexcept;
	    Plugin& operator=(const Plugin& other);
	    Plugin& operator=(Plugin&& other) noexcept;

	PLUGIFY_ACCESS:
	    Result<void> Initialize();
	    Result<void> Load(Plugify& plugify);
	    void Update();
        void Unload();
	    void Terminate();

	public:
		// Getters
		[[nodiscard]] UniqueId GetId() const noexcept;
		[[nodiscard]] std::shared_ptr<Module> GetModule() const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] PackageType GetType() const noexcept;
		[[nodiscard]] const Version& GetVersion() const noexcept;
		[[nodiscard]] const std::string& GetDescription() const noexcept;
		[[nodiscard]] const std::string& GetAuthor() const noexcept;
		[[nodiscard]] const std::string& GetWebsite() const noexcept;
		[[nodiscard]] const std::string& GetLicense() const noexcept;
		[[nodiscard]] const std::filesystem::path& GetLocation() const noexcept;
		[[nodiscard]] const std::vector<std::string>& GetPlatforms() const noexcept;
		[[nodiscard]] const std::vector<Dependency>& GetDependencies() const noexcept;
		[[nodiscard]] const std::vector<Conflict>& GetConflicts() const noexcept;
		[[nodiscard]] const std::vector<Obsolete>& GetObsoletes() const noexcept;

		// Getters
		[[nodiscard]] const std::string& GetLanguage() const noexcept;
		[[nodiscard]] const std::string& GetEntry() const noexcept;
		[[nodiscard]] const std::vector<std::string>& GetCapabilities() const noexcept;
		[[nodiscard]] const std::vector<Method>& GetMethods() const;
	    [[nodiscard]] const std::vector<MethodData>& GetMethodsData() const noexcept;

	    // Getters
	    const std::filesystem::path& GetBaseDir() const noexcept;
	    const std::filesystem::path& GetConfigsDir() const noexcept;
	    const std::filesystem::path& GetDataDir() const noexcept;
	    const std::filesystem::path& GetLogsDir() const noexcept;
	    MemAddr GetUserData() const noexcept;

		// Setters
		void SetId(UniqueId id) noexcept;
		void SetModule(std::shared_ptr<Module> module) noexcept;
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

		// Setters
		void SetLanguage(std::string language) noexcept;
		void SetEntry(std::string entry) noexcept;
		void SetCapabilities(std::vector<std::string> capabilities) noexcept;
		void SetMethods(std::vector<Method> methods) noexcept;
	    void SetMethodsData(std::vector<MethodData> methodsData) noexcept;

	    // Setters
	    void SetBaseDir(std::filesystem::path base) noexcept;
	    void SetConfigsDir(std::filesystem::path configs) noexcept;
	    void SetDataDir(std::filesystem::path data) noexcept;
	    void SetLogsDir(std::filesystem::path logs) noexcept;
	    void SetUserData(MemAddr userData) noexcept;

		[[nodiscard]] bool operator==(const Plugin& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Plugin& other) const noexcept;

		static inline constexpr std::string_view kFileExtension = ".pplugin";

	PLUGIFY_ACCESS:
	    std::unique_ptr<Impl> _impl;
	};
}