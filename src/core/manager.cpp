#include "manager.hpp"
#include "../../include/plugify/core/plugify.hpp"
#include "json.hpp"
#include "module_manifest.hpp"
#include "plugify/api/plugin_manifest_handle.hpp"
#include "plugin_manifest.hpp"
#include <plugify/core/module.hpp"
#include <plugify/core/plugin.hpp"

#include <../../include/plugify/core/plugify.hpp>
#include <plugify/api/dependency_handle.hpp>
#include <plugify/api/manager.hpp>

using namespace plugify;

struct PluginManager::Impl {
	// Configuration
	PluginManagerConfig config;

	// Discovered packages
	std::unordered_map<PackageId, ModuleInfo> modules;
	std::unordered_map<PackageId, PluginInfo> plugins;

	// Loaded instances
	std::unordered_map<PackageId, std::unique_ptr<Module>> loadedModules;
	std::unordered_map<PackageId, std::unique_ptr<Plugin>> loadedPlugins;

	// Dependency injection components
	std::unique_ptr<IPackageDiscovery> packageDiscovery;
	std::unique_ptr<IPackageValidator> packageValidator;
	std::unique_ptr<IDependencyResolver> dependencyResolver;
	std::unique_ptr<IModuleLoader> moduleLoader;
	std::unique_ptr<IPluginLoader> pluginLoader;

	// Event handling
	std::vector<EventHandler> eventHandlers;
	mutable std::mutex eventMutex;

	// Dependency tracking
	std::unordered_map<PackageId, std::vector<PackageId>> pluginToModuleMap;
	std::unordered_map<PackageId, std::vector<PackageId>> moduleToPluginsMap;
};