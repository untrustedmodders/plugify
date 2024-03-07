#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <filesystem>
#include <functional>
#include <optional>
#include <plugify_export.h>

namespace plugify {
	class IModule;
	class IPlugin;
	class PluginManager;
	struct PluginReferenceDescriptor;

	/**
	 * @brief Represents a unique identifier used for identifying modules, plugins, or other entities.
	 */
	using UniqueId = std::uintmax_t;

	/**
	 * @brief Represents a reference wrapper for a constant IModule object.
	 *        Useful when you want to pass or store a reference to an IModule without owning it.
	 */
	using ModuleRef = std::reference_wrapper<const IModule>;

	/**
	 * @brief Represents a reference wrapper for a constant IPlugin object.
	 *        Useful when you want to pass or store a reference to an IPlugin without owning it.
	 */
	using PluginRef = std::reference_wrapper<const IPlugin>;

	/**
	 * @brief Represents an optional reference to an IModule.
	 *        Used to indicate the possibility of not finding a module.
	 */
	using ModuleOpt = std::optional<ModuleRef>;

	/**
	 * @brief Represents an optional reference to an IPlugin.
	 *        Used to indicate the possibility of not finding a plugin.
	 */
	using PluginOpt = std::optional<PluginRef>;

	// Plugin manager provided to user, which implemented in core
	class PLUGIFY_API IPluginManager {
	protected:
		explicit IPluginManager(PluginManager& impl);
		~IPluginManager() = default;

	public:
		/**
		 * @brief Initialize the plugin manager.
		 * @return True if initialization is successful, false otherwise.
		 */
		bool Initialize() const;

		/**
		 * @brief Terminate the plugin manager.
		 */
		void Terminate() const;

		/**
		 * @brief Check if the plugin manager is initialized.
		 * @return True if the plugin manager is initialized, false otherwise.
		 */
		[[nodiscard]] bool IsInitialized() const;

		/**
		 * @brief Find a module by name.
		 * @param moduleName Name of the module to find.
		 * @return Optional reference to the found module.
		 */
		[[nodiscard]] ModuleOpt FindModule(const std::string& moduleName) const;

		/**
		 * @brief Find a module by name (string view version).
		 * @param moduleName Name of the module to find.
		 * @return Optional reference to the found module.
		 */
		[[nodiscard]] ModuleOpt FindModule(std::string_view moduleName) const;

		/**
		 * @brief Find a module by unique identifier.
		 * @param moduleId Unique identifier of the module to find.
		 * @return Optional reference to the found module.
		 */
		[[nodiscard]] ModuleOpt FindModuleFromId(UniqueId moduleId) const;

		/**
		 * @brief Find a module by language.
		 * @param moduleLang Language of the module to find.
		 * @return Optional reference to the found module.
		 */
		[[nodiscard]] ModuleOpt FindModuleFromLang(const std::string& moduleLang) const;

		/**
		 * @brief Find a module by file path.
		 * @param moduleFilePath File path of the module to find.
		 * @return Optional reference to the found module.
		 */
		[[nodiscard]] ModuleOpt FindModuleFromPath(const std::filesystem::path& moduleFilePath) const;

		/**
		 * @brief Get a vector of references to all modules.
		 * @return Vector of module references.
		 */
		[[nodiscard]] std::vector<ModuleRef> GetModules() const;

		/**
		 * @brief Find a plugin by name.
		 * @param pluginName Name of the plugin to find.
		 * @return Optional reference to the found plugin.
		 */
		[[nodiscard]] PluginOpt FindPlugin(const std::string& pluginName) const;

		/**
		 * @brief Find a plugin by name (string view version).
		 * @param pluginName Name of the plugin to find.
		 * @return Optional reference to the found plugin.
		 */
		[[nodiscard]] PluginOpt FindPlugin(std::string_view pluginName) const;

		/**
		 * @brief Find a plugin by unique identifier.
		 * @param pluginId Unique identifier of the plugin to find.
		 * @return Optional reference to the found plugin.
		 */
		[[nodiscard]] PluginOpt FindPluginFromId(UniqueId pluginId) const;

		/**
		 * @brief Find a plugin by its descriptor.
		 * @param pluginDescriptor Descriptor of the plugin to find.
		 * @return Optional reference to the found plugin.
		 */
		[[nodiscard]] PluginOpt FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) const;

		/**
		 * @brief Get a vector of references to all plugins.
		 * @return Vector of plugin references.
		 */
		[[nodiscard]] std::vector<PluginRef> GetPlugins() const;

		/**
		 * @brief Get the dependencies of a specific plugin.
		 * @param pluginName Name of the plugin to query.
		 * @param pluginDependencies Vector to store the dependencies.
		 * @return True if dependencies are retrieved successfully, false otherwise.
		 */
		bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;

		/**
		 * @brief Get the dependencies of a plugin using its descriptor.
		 * @param pluginDescriptor Descriptor of the plugin to query.
		 * @param pluginDependencies Vector to store the dependencies.
		 * @return True if dependencies are retrieved successfully, false otherwise.
		 */
		bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) const;

	private:
		PluginManager& _impl; ///< The implementation of the plugin manager.
	};
} // namespace plugify
