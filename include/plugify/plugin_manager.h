#pragma once

#include <cstdint>
#include <string>
#include <filesystem>
#include <functional>
#include <optional>
#include <plugify_export.h>

namespace plugify {
	class ModuleRef;
	class PluginRef;
	class PluginReferenceDescriptorRef;

	/**
	 * @brief Represents a unique identifier used for identifying modules, plugins, or other entities.
	 */
	using UniqueId = std::ptrdiff_t;

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

	/**
	 * @class IPluginManager
	 * @brief Interface for the plugin manager provided to the user, implemented in the core.
	 */
	class IPluginManager {
	public:
		virtual ~IPluginManager() = default;

		/**
		 * @brief Initialize the plugin manager.
		 * @return True if initialization is successful, false otherwise.
		 */
		virtual bool Initialize() = 0;

		/**
		 * @brief Terminate the plugin manager.
		 */
		virtual void Terminate() = 0;

		/**
		 * @brief Check if the plugin manager is initialized.
		 * @return True if the plugin manager is initialized, false otherwise.
		 */
		[[nodiscard]] virtual bool IsInitialized() const = 0;

		/**
		 * @brief Find a module by name.
		 * @param moduleName Name of the module to find.
		 * @return Optional reference to the found module.
		 */
		[[nodiscard]] virtual ModuleOpt FindModule(std::string_view moduleName) const = 0;

		/**
		 * @brief Find a module by unique identifier.
		 * @param moduleId Unique identifier of the module to find.
		 * @return Optional reference to the found module.
		 */
		[[nodiscard]] virtual ModuleOpt FindModuleFromId(UniqueId moduleId) const = 0;

		/**
		 * @brief Find a module by language.
		 * @param moduleLang Language of the module to find.
		 * @return Optional reference to the found module.
		 */
		[[nodiscard]] virtual ModuleOpt FindModuleFromLang(const std::string_view moduleLang) const = 0;

		/**
		 * @brief Find a module by file path.
		 * @param moduleFilePath File path of the module to find.
		 * @return Optional reference to the found module.
		 */
		[[nodiscard]] virtual ModuleOpt FindModuleFromPath(const std::filesystem::path& moduleFilePath) const = 0;

		/**
		 * @brief Get a vector of references to all modules.
		 * @return Vector of module references.
		 */
		[[nodiscard]] virtual std::vector<ModuleRef> GetModules() const = 0;

		/**
		 * @brief Find a plugin by name.
		 * @param pluginName Name of the plugin to find.
		 * @return Optional reference to the found plugin.
		 */
		[[nodiscard]] virtual PluginOpt FindPlugin(std::string_view pluginName) const = 0;

		/**
		 * @brief Find a plugin by unique identifier.
		 * @param pluginId Unique identifier of the plugin to find.
		 * @return Optional reference to the found plugin.
		 */
		[[nodiscard]] virtual PluginOpt FindPluginFromId(UniqueId pluginId) const = 0;

		/**
		 * @brief Find a plugin by its descriptor.
		 * @param pluginDescriptor Descriptor of the plugin to find.
		 * @return Optional reference to the found plugin.
		 */
		[[nodiscard]] virtual PluginOpt FindPluginFromDescriptor(const PluginReferenceDescriptorRef& pluginDescriptor) const = 0;

		/**
		 * @brief Get a vector of references to all plugins.
		 * @return Vector of plugin references.
		 */
		[[nodiscard]] virtual std::vector<PluginRef> GetPlugins() const = 0;
	};
} // namespace plugify
