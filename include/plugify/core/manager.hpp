#pragma once

#include <plugify/core/context.hpp>

namespace plugify {
	class Plugify;
	class Manager : public Context {
	public:
		explicit Manager(Plugify& plugify);
		~Manager() override;

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
		GetModule(std::string_view moduleId) const override;
		std::optional<std::reference_wrapper<const PluginInfo>>
		GetPlugin(std::string_view pluginId) const override;

		std::vector<std::reference_wrapper<const ModuleInfo>>
		GetModules() const override;
		std::vector<std::reference_wrapper<const PluginInfo>>
		GetPlugins() const override;

		bool IsModuleLoaded(std::string_view moduleId) const override;
		bool IsPluginLoaded(std::string_view pluginId) const override;

		void Subscribe(EventHandler handler) override;
		void Unsubscribe(EventHandler handler) override;

	private:
		struct Impl;
		std::unique_ptr<Impl> pImpl;

		// Internal initialization steps
		Result<void> ValidateAllManifests();
		Result<void> ResolveDependencies();
		Result<void> InitializeModules();
		Result<void> InitializePlugins();

		// Helper methods
		void EmitEvent(Event event);
		Result<void> HandleInitializationError(const PackageId& id, const Error& error);

		// State management
		void UpdatePackageState(const PackageId& id, PackageState newState);
		std::vector<PackageId> GetPluginsForModule(std::string_view moduleId) const;
	};
}