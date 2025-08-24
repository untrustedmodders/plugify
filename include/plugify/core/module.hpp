#pragma once
#if 0
#include "plugify/../../src/core/module_manifest.hpp"
#include "plugify/_/module_handle.hpp"
#include "date_time.hpp"
#include "language_module.hpp"
#include "plugify/asm/assembly.hpp"
#include "plugify/core/dependency.hpp"

namespace plugify {
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
		UniqueId GetId() const noexcept {
			return _id;
		}

		static std::string_view GetType() noexcept {
			return "Module";
		}

		std::string_view GetName() const noexcept {
			return _manifest->name;
		}

		const Version& GetVersion() const noexcept {
			return _manifest->version;
		}

		std::span<const std::unique_ptr<Dependency>> GetDependencies() const noexcept {
			return _manifest->dependencies ? *_manifest->dependencies : std::span<const std::unique_ptr<Dependency>>{};
		}

		std::span<const std::unique_ptr<Conflict>> GetConflicts() const noexcept {
			return _manifest->conflicts ? *_manifest->conflicts : std::span<const std::unique_ptr<Conflict>>{};
		}

		std::string_view GetLanguage() const noexcept {
			return _manifest->language;
		}

		const std::filesystem::path& GetBaseDir() const noexcept {
			return _paths.base;
		}

		const ModuleManifest& GetManifest() const noexcept {
			return *_manifest;
		}

		ModuleState GetState() const noexcept {
			return _state;
		}

		std::string_view GetError() const noexcept {
			return _error;
		}

		bool Initialize(Plugify& plugify);
		void Terminate();
		void Update(DateTime dt);

		bool LoadPlugin(Plugin& plugin) const;
		void StartPlugin(Plugin& plugin) const;
		void UpdatePlugin(Plugin& plugin, DateTime dt) const;
		void EndPlugin(Plugin& plugin) const;
		void MethodExport(Plugin& plugin) const;

		void SetError(std::string_view error);

		ILanguageModule* GetLanguageModule() const {
			return _languageModule;
		}

		void SetLoaded() noexcept {
			_state = ModuleState::Loaded;
		}

		void SetUnloaded() noexcept {
			_state = ModuleState::NotLoaded;
		}

		Module& operator=(const Module&) = delete;
		Module& operator=(Module&& other) noexcept = default;

	private:
		ILanguageModule* _languageModule{ nullptr };
		ModuleState _state{ ModuleState::NotLoaded };
		MethodTable _table;
		UniqueId _id;
		BasePaths _paths;
		std::shared_ptr<ModuleManifest> _manifest;
		std::unique_ptr<IAssembly> _assembly;
		std::string_view _error;
		mutable std::vector<PluginHandle> _loadedPlugins; // debug only
	};

	class Dependency;
	class Conflict;

	// Module Class
	class PLUGIFY_API Module {
	protected:
	    struct Impl;

	public:
	    Module();
	    ~Module();
	    Module(const Module& other);
	    Module(Module&& other) noexcept;
	    Module& operator=(const Module& other);
	    Module& operator=(Module&& other) noexcept;

	    // Getters
	    [[nodiscard]] const UniqueId& GetId() const noexcept;
	    [[nodiscard]] std::string_view GetName() const noexcept;
	    [[nodiscard]] PackageType GetType() const noexcept;
	    [[nodiscard]] Version GetVersion() const noexcept;
	    [[nodiscard]] std::string_view GetDescription() const noexcept;
	    [[nodiscard]] std::string_view GetAuthor() const noexcept;
	    [[nodiscard]] std::string_view GetWebsite() const noexcept;
	    [[nodiscard]] std::string_view GetLicense() const noexcept;
	    [[nodiscard]] const std::filesystem::path& GetLocation() const noexcept;
	    [[nodiscard]] std::span<const std::string> GetPlatforms() const noexcept;
	    [[nodiscard]] std::span<const Dependency> GetDependencies() const;
	    [[nodiscard]] std::span<const Conflict> GetConflicts() const;
	    [[nodiscard]] std::span<const Obsolete> GetObsoletes() const;

		// Getters
		[[nodiscard]] std::string_view GetLanguage() const noexcept;
		[[nodiscard]] const std::filesystem::path& GetRuntime() const noexcept;
		[[nodiscard]] std::span<const std::string> GetDirectories() const noexcept;
		[[nodiscard]] bool GetForceLoad() const noexcept;

	    // Setters
	    void SetId(UniqueId id) noexcept;
	    void SetName(std::string_view name) noexcept;
	    void SetType(PackageType type) noexcept;
	    void SetVersion(Version version) noexcept;
	    void SetDescription(std::string_view description) noexcept;
	    void SetAuthor(std::string_view author) noexcept;
	    void SetWebsite(std::string_view website) noexcept;
	    void SetLicense(std::string_view license) noexcept;
	    void SetLocation(std::filesystem::path location) noexcept;
	    void SetPlatforms(std::span<const std::string> platforms) noexcept;
	    void SetDependencies(std::span<const Dependency> dependencies) noexcept;
	    void SetConflicts(std::span<const Conflict> conflicts) noexcept;
	    void SetObsoletes(std::span<const Obsolete> obsoletes) noexcept;

		// Setters (pass by value and move)
		void SetLanguage(std::string_view language) noexcept;
		void SetRuntime(std::filesystem::path runtimePath) noexcept;
		void SetDirectories(std::span<const std::string> directories) noexcept;
		void SetForceLoad(bool forceLoad) noexcept;

		[[nodiscard]] bool operator==(const Module& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Module& other) const noexcept;

		static inline constexpr std::string_view kFileExtension = "*.pmodule";
		static inline constexpr std::string_view kGetLanguageModuleFn = "GetLanguageModule";

	protected:
	    //friend struct glz::meta<PackageManifest>;
	    //friend struct std::hash<PackageManifest>;
	    std::unique_ptr<Impl> impl;
	};
}
#endif