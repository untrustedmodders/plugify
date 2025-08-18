
#include <concepts>
#include <expected>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace PluginSystem {

	// Forward declarations
	class Module;
	class Plugin;

	// ============================================================================
	// Core Types and Concepts
	// ============================================================================

	using PackageId = std::string;
	using Version = std::string;
	using LanguageType = std::string;
	using ErrorMessage = std::string;

	// Error types for better error handling
	enum class ErrorCode {
		None,
		FileNotFound,
		InvalidManifest,
		MissingDependency,
		VersionConflict,
		InitializationFailed,
		LanguageModuleNotLoaded,
		CircularDependency,
		ValidationFailed,
		DisabledByPolicy
	};

	struct Error {
		ErrorCode code;
		ErrorMessage message;
		std::optional<std::filesystem::path> source;
	};

	template<typename T>
	using Result = std::expected<T, Error>;

	// ============================================================================
	// Package Metadata Structures
	// ============================================================================

	struct Dependency {
		std::string name;
		std::optional<std::vector<Constraint>> constraints;// ANDed together
		std::optional<bool> optional;

		std::vector<Constraint> GetFailedConstraints(const Version& version) const {
			if (!constraints) return {};
			std::vector<Constraint> failed;
			std::ranges::copy_if(*constraints, std::back_inserter(failed), [&](const Constraint& c) {
				return !c.IsSatisfiedBy(version);
			});
			return failed;
		}
	};

	struct Conflict {
		std::string name;
		std::optional<std::vector<Constraint>> constraints;// Versions that conflict
		std::optional<std::string> reason;

		std::vector<Constraint> GetSatisfiedConstraints(const Version& version) const {
			if (!constraints) return {};
			std::vector<Constraint> satisfied;
			std::ranges::copy_if(*constraints, std::back_inserter(satisfied), [&](const Constraint& c) {
				return c.IsSatisfiedBy(version);
			});
			return satisfied;
		}

		bool operator==(const Conflict& conflict) const noexcept = default;
	};

	struct PackageManifest {
		PackageId id;
		std::string name;
		Version version;
		std::string description;
		std::vector<std::string> authors;
		std::filesystem::path location;

		// Dependencies and conflicts
		std::vector<Dependency> dependencies;
		std::unordered_set<PackageId> conflicts;

		// Metadata
		std::unordered_map<std::string, std::string> metadata;
	};

	struct PluginManifest : PackageManifest {
		LanguageType language;
		std::string entryPoint;
		std::vector<std::string> capabilities;
	};

	struct ModuleManifest : PackageManifest {
		LanguageType languageType;
		std::filesystem::path runtimePath;
		std::vector<std::string> supportedVersions;
	};

	// ============================================================================
	// Package State Management
	// ============================================================================

	enum class PackageState {
		Discovered,
		Validated,
		Initializing,
		Ready,
		Started,
		Ended,
		Error,
		Disabled
	};

	template<typename ManifestType>
	struct PackageInfo {
		ManifestType manifest;
		PackageState state = PackageState::Discovered;
		std::optional<Error> lastError;
		std::chrono::system_clock::time_point discoveredAt;
	};

	using PluginInfo = PackageInfo<PluginManifest>;
	using ModuleInfo = PackageInfo<ModuleManifest>;

	// ============================================================================
	// Interfaces
	// ============================================================================

	// Package discovery interface
	class IPackageDiscovery {
	public:
		virtual ~IPackageDiscovery() = default;

		virtual Result<std::vector<ModuleInfo>> discoverModules(
				std::span<const std::filesystem::path> searchPaths) = 0;

		virtual Result<std::vector<PluginInfo>> discoverPlugins(
				std::span<const std::filesystem::path> searchPaths) = 0;
	};

	// Package validation interface
	class IPackageValidator {
	public:
		virtual ~IPackageValidator() = default;

		virtual Result<void> validateManifest(const PackageManifest& manifest) = 0;
		virtual Result<void> validateDependencies(
				const PackageManifest& package,
				std::span<const PackageManifest> availablePackages) = 0;
		virtual Result<void> checkConflicts(
				const PackageManifest& package,
				std::span<const PackageManifest> loadedPackages) = 0;
	};

	// Dependency resolution interface
	class IDependencyResolver {
	public:
		virtual ~IDependencyResolver() = default;

		struct Resolution {
			std::vector<PackageId> loadOrder;
			std::unordered_map<PackageId, std::vector<PackageId>> dependencyGraph;
			std::vector<Error> warnings;
		};

		virtual Result<Resolution> resolve(
				std::span<const PluginInfo> plugins,
				std::span<const ModuleInfo> modules) = 0;
	};

	// Module loader interface
	class IModuleLoader {
	public:
		virtual ~IModuleLoader() = default;

		virtual Result<std::unique_ptr<Module>> loadModule(
				const ModuleManifest& manifest) = 0;
		virtual Result<void> initializeModule(
				Module& module,
				const ModuleManifest& manifest) = 0;
		virtual void unloadModule(std::unique_ptr<Module> module) = 0;
	};

	// Plugin loader interface
	class IPluginLoader {
	public:
		virtual ~IPluginLoader() = default;

		virtual Result<std::unique_ptr<Plugin>> loadPlugin(
				const PluginManifest& manifest,
				Module& languageModule) = 0;
		virtual Result<void> initializePlugin(
				Plugin& plugin,
				const PluginManifest& manifest) = 0;
		virtual void unloadPlugin(std::unique_ptr<Plugin> plugin) = 0;
	};

	// ============================================================================
	// Event System for Monitoring
	// ============================================================================

	enum class EventType {
		ModuleDiscovered,
		PluginDiscovered,

		ModuleLoading,
		ModuleLoaded,
		ModuleFailed,

		PluginLoading,
		PluginLoaded,
		PluginFailed,

		PluginStarted,
		PluginStopped,

		ConflictDetected,
		DependencyResolved
	};

	struct Event {
		EventType type;
		std::chrono::system_clock::time_point timestamp;
		PackageId packageId;
		std::optional<Error> error;
		std::unordered_map<std::string, std::string> data;
	};

	using EventHandler = std::function<void(const Event&)>;

	// ============================================================================
	// Main PluginManager Interface
	// ============================================================================

	class IPluginManager {
	public:
		virtual ~IPluginManager() = default;

		// Discovery
		virtual Result<void> discoverPackages(
				std::span<const std::filesystem::path> searchPaths) = 0;

		// Initialization sequence
		virtual Result<void> initialize() = 0;

		// Package queries
		virtual std::optional<std::reference_wrapper<const ModuleInfo>>
		getModule(std::string_view moduleId) const = 0;
		virtual std::optional<std::reference_wrapper<const PluginInfo>>
		getPlugin(std::string_view pluginId) const = 0;

		virtual std::vector<std::reference_wrapper<const ModuleInfo>>
		getModules() const = 0;
		virtual std::vector<std::reference_wrapper<const PluginInfo>>
		getPlugins() const = 0;

		// State queries
		virtual bool isModuleLoaded(std::string_view moduleId) const = 0;
		virtual bool isPluginLoaded(std::string_view pluginId) const = 0;

		// Event handling
		virtual void subscribe(EventHandler handler) = 0;
		virtual void unsubscribe(EventHandler handler) = 0;
	};

	// ============================================================================
	// Configuration
	// ============================================================================

	struct PluginManagerConfig {
		std::vector<std::filesystem::path> searchPaths;
		bool autoResolveConflicts = false;
		bool strictVersionChecking = true;
		bool loadDisabledPackages = false;
		std::size_t maxInitializationRetries = 3;
		std::chrono::milliseconds initializationTimeout{30000};

		// Filtering

	};

	// ============================================================================
	// Concrete Implementation
	// ============================================================================

	class PluginManager : public IPluginManager {
	public:
		explicit PluginManager(PluginManagerConfig config);
		~PluginManager() override;

		// Dependency injection for customization
		void setPackageDiscovery(std::unique_ptr<IPackageDiscovery> discovery);
		void setPackageValidator(std::unique_ptr<IPackageValidator> validator);
		void setDependencyResolver(std::unique_ptr<IDependencyResolver> resolver);
		void setModuleLoader(std::unique_ptr<IModuleLoader> loader);
		void setPluginLoader(std::unique_ptr<IPluginLoader> loader);

		// IPluginManager implementation
		Result<void> discoverPackages(
				std::span<const std::filesystem::path> searchPaths) override;
		Result<void> initialize() override;

		std::optional<std::reference_wrapper<const ModuleInfo>>
		getModule(std::string_view moduleId) const override;
		std::optional<std::reference_wrapper<const PluginInfo>>
		getPlugin(std::string_view pluginId) const override;

		std::vector<std::reference_wrapper<const ModuleInfo>>
		getModules() const override;
		std::vector<std::reference_wrapper<const PluginInfo>>
		getPlugins() const override;

		bool isModuleLoaded(std::string_view moduleId) const override;
		bool isPluginLoaded(std::string_view pluginId) const override;

		void subscribe(EventHandler handler) override;
		void unsubscribe(EventHandler handler) override;

	private:
		struct Impl;
		std::unique_ptr<Impl> pImpl;

		// Internal initialization steps
		Result<void> validateAllManifests();
		Result<void> resolveDependencies();
		Result<void> initializeModules();
		Result<void> initializePlugins();

		// Helper methods
		void emitEvent(Event event);
		Result<void> handleInitializationError(const PackageId& id, const Error& error);

		// State management
		void updatePackageState(const PackageId& id, PackageState newState);
		std::vector<PackageId> getPluginsForModule(std::string_view moduleId) const;
	};

	// ============================================================================
	// Factory for Creating Default Components
	// ============================================================================

	class PluginManagerFactory {
	public:
		static std::unique_ptr<IPluginManager> create(PluginManagerConfig config);

		static std::unique_ptr<IPackageDiscovery> createDefaultDiscovery();
		static std::unique_ptr<IPackageValidator> createDefaultValidator();
		static std::unique_ptr<IDependencyResolver> createDefaultResolver();
		static std::unique_ptr<IModuleLoader> createDefaultModuleLoader();
		static std::unique_ptr<IPluginLoader> createDefaultPluginLoader();
	};

	// ============================================================================
	// Utility Functions for Package Management
	// ============================================================================

	namespace Utils {
		// Version comparison utilities
		bool isVersionCompatible(const Version& required,
								 const Version& available);
		std::strong_ordering compareVersions(const Version& v1,
											 const Version& v2);

		// Manifest utilities
		Result<PluginManifest> parsePluginManifest(
				const std::filesystem::path& path);
		Result<ModuleManifest> parseModuleManifest(
				const std::filesystem::path& path);

		// Dependency graph utilities
		template<typename PackageType>
		std::vector<PackageId> topologicalSort(
				std::span<const PackageType> packages);

		template<typename PackageType>
		bool hasCyclicDependency(
				std::span<const PackageType> packages);

		// Filtering utilities using ranges
		template<typename Container>
		auto filterByState(Container&& packages, PackageState state) {
			return packages | std::views::filter([state](const auto& pkg) {
					   return pkg.state == state;
				   });
		}

		template<typename Container>
		auto filterByLanguage(Container&& plugins, std::string_view language) {
			return plugins | std::views::filter([language](const auto& plugin) {
					   return plugin.manifest.language == language;
				   });
		}
	}// namespace Utils

}// namespace PluginSystem``` In my system plugins should be load by language modules