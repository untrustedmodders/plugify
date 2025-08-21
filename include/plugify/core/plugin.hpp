#pragma once

#if 0
#include "plugify/core/conflict.hpp"
#include "plugify/core/dependency.hpp"

namespace plugify {
	class Plugify;
	class Module;
	class Plugin {
	public:
		using State = PluginState;
		using Utils = PluginUtils;

		Plugin(UniqueId id, BasePaths paths, std::shared_ptr<Manifest> manifest);
		Plugin(const Plugin& plugin) = delete;
		Plugin(Plugin&& plugin) noexcept;
		~Plugin() = default;

	public:
		UniqueId GetId() const noexcept {
			return _id;
		}

		static std::string_view GetType() noexcept {
			return "Plugin";
		}

		const std::string& GetName() const noexcept {
			return _manifest->name;
		}

		const Version& GetVersion() const noexcept {
			return _manifest->version;
		}

		std::span<const Dependency> GetDependencies() const noexcept {
			return _manifest->dependencies ? *_manifest->dependencies : std::span<const Dependency>{};
		}

		std::span<const Conflict> GetConflicts() const noexcept {
			return _manifest->conflicts ? *_manifest->conflicts : std::span<const Conflict>{};
		}

		const std::filesystem::path& GetBaseDir() const noexcept {
			return _paths.base;
		}

		const std::filesystem::path& GetConfigsDir() const noexcept {
			return _paths.configs;
		}

		const std::filesystem::path& GetDataDir() const noexcept {
			return _paths.data;
		}

		const std::filesystem::path& GetLogsDir() const noexcept {
			return _paths.logs;
		}

		const PluginManifest& GetManifest() const noexcept {
			return *_manifest;
		}

		PluginState GetState() const noexcept {
			return _state;
		}

		std::span<const MethodData> GetMethods() const noexcept {
			return _methods;
		}

		MemAddr GetData() const noexcept {
			return _data;
		}

		const std::string& GetError() const noexcept {
			return _error;
		}

		void SetError(std::string error);

		void SetMethods(std::vector<MethodData> methods) {
			_methods = std::move(methods);
		}

		void SetData(MemAddr data) {
			_data = data;
		}

		void SetTable(MethodTable table) {
			_table = table;
		}

		Module* GetModule() const {
			return _module;
		}

		void SetModule(Module& module) noexcept {
			_module = &module;
		}

		void SetLoaded() noexcept {
			_state = PluginState::Loaded;
		}

		void SetRunning() noexcept {
			_state = PluginState::Running;
		}

		void SetTerminating() noexcept {
			_state = PluginState::Terminating;
		}

		void SetUnloaded() noexcept {
			_state = PluginState::NotLoaded;
		}

		bool HasUpdate() const noexcept {
			return _table.hasUpdate;
		}

		bool HasStart() const noexcept {
			return _table.hasStart;
		}

		bool HasEnd() const noexcept {
			return _table.hasEnd;
		}

		bool HasExport() const noexcept {
			return _table.hasExport;
		}

		bool Initialize(Plugify&);
		void Terminate();

		Plugin& operator=(const Plugin&) = delete;
		Plugin& operator=(Plugin&& other) noexcept = default;

	private:
		Module* _module{ nullptr };
		PluginState _state{ PluginState::NotLoaded };
		MethodTable _table;
		UniqueId _id;
		MemAddr _data;
		BasePaths _paths;
		std::vector<MethodData> _methods;
		std::shared_ptr<PluginManifest> _manifest;
		std::string _error;
	};

	// Plugin Class
	class PLUGIFY_API Plugin {
	protected:
	    struct Impl;

	public:
	    Plugin();
	    ~Plugin();
	    Plugin(const Plugin& other);
	    Plugin(Plugin&& other) noexcept;
	    Plugin& operator=(const Plugin& other);
	    Plugin& operator=(Plugin&& other) noexcept;

		// Getters
		[[nodiscard]] std::string_view GetId() const noexcept;
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

		// Getters
		[[nodiscard]] const Dependency& GetLanguage() const noexcept;
		[[nodiscard]] std::string_view GetEntry() const noexcept;
		[[nodiscard]] std::span<const std::string> GetCapabilities() const noexcept;
		[[nodiscard]] std::span<const Method> GetMethods() const;

		// Setters
		void SetId(std::string_view id) noexcept;
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

		// Setters
		void SetLanguage(const Dependency& language) noexcept;
		void SetEntry(std::string_view entry) noexcept;
		void SetCapabilities(std::span<const std::string> capabilities) noexcept;
		void SetMethods(std::span<const Method> methods) noexcept;

		[[nodiscard]] bool operator==(const Plugin& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Plugin& other) const noexcept;

		static inline constexpr std::string_view kFileExtension = ".pplugin";

	protected:
	    //friend struct glz::meta<Plugin>;
	    //friend struct std::hash<Plugin>;
	    std::unique_ptr<Impl> _impl;
	};
}
#endif