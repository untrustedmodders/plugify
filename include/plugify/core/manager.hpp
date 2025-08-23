#pragma once

#include "plugify/core/event.hpp"
#include "plugify/core/report.hpp"
#include "plugify/core/types.hpp"

#include "dependency_resolver.hpp"

namespace plugify {
    class IDependencyResolver;
	/*class Module;
	class Plugin;
	class IPackageDiscovery;
	class IPackageValidator;
	class IModuleLoader;
	class IPluginLoader;

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

	// Module loader interface
	class IModuleLoader {
	public:
		virtual ~IModuleLoader() = default;

		virtual Result<std::unique_ptr<Module>> loadModule(const ModuleManifest& manifest) = 0;
		virtual Result<void> initializeModule(
			Module& module,
			const ModuleManifest& manifest) = 0;
		virtual void unloadModule(
			std::unique_ptr<Module> module) = 0;
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
		virtual void unloadPlugin(
			std::unique_ptr<Plugin> plugin) = 0;
	};

	class Plugify;
	class Manager {
	public:
		explicit Manager(Plugify& plugify);
		~Manager();

		// Dependency injection for customization
		//void setPackageDiscovery(std::unique_ptr<IPackageValidator> discovery);
		//void setPackageValidator(std::unique_ptr<IPackageValidator> validator);
		void SetDependencyResolver(std::unique_ptr<IDependencyResolver> resolver);
		//void setModuleLoader(std::unique_ptr<IModuleLoader> loader);
		//void setPluginLoader(std::unique_ptr<IPluginLoader> loader);

		State<void> DiscoverPackages(std::span<const std::filesystem::path> searchPaths = {});

		State<InitializationState> Initialize();
		State<void> Terminate();
		State<void> Update() { return {}; }

		ModuleInfo GetModule(std::string_view moduleId) const;
		PluginInfo GetPlugin(std::string_view pluginId) const;

		std::vector<ModuleInfo> GetModules() const;
		std::vector<PluginInfo> GetPlugins() const;

		std::vector<ModuleInfo> GetModules(PackageState state) const;
		std::vector<PluginInfo> GetPlugins(PackageState state) const;

		bool IsModuleLoaded(std::string_view moduleId) const;
		bool IsPluginLoaded(std::string_view pluginId) const;

		UniqueId Subscribe(EventHandler handler);
		void Unsubscribe(UniqueId id);

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;

		// Internal initialization steps
		ValidationReport ValidateAllManifests();
		DependencyReport ResolveDependencies();
		InitializationReport InitializeModules();
		InitializationReport InitializePlugins();

		// Helper methods
		void EmitEvent(const Event& event);
		//Result<void> HandleInitializationError(const PackageId& id, const Error& error);

		// State management
		void UpdatePackageState(const PackageId& id, PackageState newState);
		std::vector<PackageId> GetPluginsForModule(std::string_view moduleId) const;
	};

    class ManagerFactory {
    public:
        //static std::unique_ptr<IPackageDiscovery> createDefaultDiscovery();
        //static std::unique_ptr<IPackageValidator> createDefaultValidator();
        static std::unique_ptr<IDependencyResolver> CreateDefaultResolver();
        //static std::unique_ptr<IModuleLoader> createDefaultModuleLoader();
        //static std::unique_ptr<IPluginLoader> createDefaultPluginLoader();
    };*/
}