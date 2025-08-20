#pragma once

#include "plugify/core/types.hpp"
#include "plugify/core/event.hpp"
#include "plugify/core/context.hpp"
#include "plugify/core/report.hpp"

namespace plugify {
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
		~Manager() override;

		// Dependency injection for customization
		void setPackageDiscovery(std::unique_ptr<IPackageValidator> discovery);
		void setPackageValidator(std::unique_ptr<IPackageValidator> validator);
		void setDependencyResolver(std::unique_ptr<?> resolver);
		void setModuleLoader(std::unique_ptr<IModuleLoader> loader);
		void setPluginLoader(std::unique_ptr<IPluginLoader> loader);

		// IPluginManager implementation
		Result<void> DiscoverPackages(std::span<const std::filesystem::path> searchPaths) override;
		Result<void> initialize() override;

		std::optional<std::reference_wrapper<const ModuleInfo>>
		GetModule(std::string_view moduleId) const override;
		std::optional<std::reference_wrapper<const PluginInfo>>
		GetPlugin(std::string_view pluginId) const override;

		std::vector<std::reference_wrapper<const ModuleInfo>>
		GetModules() const override;
		std::vector<std::reference_wrapper<const PluginInfo>>
		GetPlugins() const override;

		bool IsModuleLoaded(std::string_view moduleId) const override;
		bool IsPluginLoaded(std::string_view pluginId) const override;

		UniqueId Subscribe(EventHandler handler) override;
		void Unsubscribe(UniqueId token) override;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;

		// Internal initialization steps
		ValidationReport ValidateAllManifests();
		DependencyReport ResolveDependencies();
		InitializationReport InitializeModules();
		InitializationReport InitializePlugins();

		// Helper methods
		void EmitEvent(Event event);
		Result<void> HandleInitializationError(const PackageId& id, const Error& error);

		// State management
		void UpdatePackageState(const PackageId& id, PackageState newState);
		std::vector<PackageId> GetPluginsForModule(std::string_view moduleId) const;
	};
}