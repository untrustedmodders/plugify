#pragma once

#include "plugify/core/manifest.hpp"

namespace plugify {
#if 0
#include "plugify/../../src/core/module_manifest.hpp"
#include "plugify/_/module_handle.hpp"
#include "date_time.hpp"
#include "language_module.hpp"
#include "plugify/asm/assembly.hpp"
#include "plugify/core/dependency.hpp"

	struct Manifest;
	class Plugin;
	class Plugify;
	class Module {
	public:
		using State = ModuleState;
		using Utils = ModuleUtils;

		Module(UniqueId id, BasePaths paths, std::shared_ptr<Manifest> manifest);
		Module(const Module& module) = delete;
		Module(Module&& module) noexcept;
		~Module() = default;

	public:

		std::span<const std::unique_ptr<Dependency>> GetDependencies() const noexcept {
			return _manifest->dependencies ? *_manifest->dependencies : std::span<const std::unique_ptr<Dependency>>{};
		}

		std::span<const std::unique_ptr<Conflict>> GetConflicts() const noexcept {
			return _manifest->conflicts ? *_manifest->conflicts : std::span<const std::unique_ptr<Conflict>>{};
		}

		const std::string& GetLanguage() const noexcept {
			return _manifest->language;
		}

		const std::filesystem::path& GetBaseDir() const noexcept {
			return _paths.base;
		}






		void SetError(std::string error);

		ILanguageModule* GetLanguageModule() const {
			return _languageModule;
		}


		Module& operator=(const Module&) = delete;
		Module& operator=(Module&& other) noexcept = default;

	private:
		UniqueId _id;
		BasePaths _paths;
		std::shared_ptr<ModuleManifest> _manifest;
		std::string_view _error;
		mutable std::vector<PluginHandle> _loadedPlugins; // debug only
	};
#endif
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