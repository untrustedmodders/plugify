#pragma once

#include <cstdint>
#include <string>
#include <filesystem>
#include <functional>
#include <plugify/date_time.hpp>
#include <plugify_export.h>

namespace plugify {
	class ModuleHandle;
	class PluginHandle;
	class PluginReferenceDescriptorHandle;

	/**
	 * @brief Represents a unique identifier used for identifying modules, plugins, or other entities.
	 */
	using UniqueId = std::ptrdiff_t;

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
		virtual bool IsInitialized() const = 0;

		/**
		 * @brief Updates the package manager.
		 * @param dt The time delta since the last update.
		 */
		virtual void Update(DateTime dt) = 0;

		/**
		 * @brief Find a module by name.
		 * @param moduleName Name of the module to find.
		 * @return Handle to the found module.
		 */
		virtual ModuleHandle FindModule(std::string_view moduleName) const = 0;

		/**
		 * @brief Find a module by unique identifier.
		 * @param moduleId Unique identifier of the module to find.
		 * @return Handle to the found module.
		 */
		virtual ModuleHandle FindModuleFromId(UniqueId moduleId) const = 0;

		/**
		 * @brief Find a module by language.
		 * @param moduleLang Language of the module to find.
		 * @return Handle to the found module.
		 */
		virtual ModuleHandle FindModuleFromLang(std::string_view moduleLang) const = 0;

		/**
		 * @brief Find a module by file path.
		 * @param moduleFilePath File path of the module to find.
		 * @return Handle to the found module.
		 */
		virtual ModuleHandle FindModuleFromPath(const std::filesystem::path& moduleFilePath) const = 0;

		/**
		 * @brief Get a vector of handles to all modules.
		 * @return Vector of module handles.
		 */
		virtual std::vector<ModuleHandle> GetModules() const = 0;

		/**
		 * @brief Find a plugin by name.
		 * @param pluginName Name of the plugin to find.
		 * @return Handle to the found plugin.
		 */
		virtual PluginHandle FindPlugin(std::string_view pluginName) const = 0;

		/**
		 * @brief Find a plugin by unique identifier.
		 * @param pluginId Unique identifier of the plugin to find.
		 * @return Handle to the found plugin.
		 */
		virtual PluginHandle FindPluginFromId(UniqueId pluginId) const = 0;

		/**
		 * @brief Find a plugin by its descriptor.
		 * @param pluginDescriptor Descriptor of the plugin to find.
		 * @return Handle to the found plugin.
		 */
		virtual PluginHandle FindPluginFromDescriptor(const PluginReferenceDescriptorHandle & pluginDescriptor) const = 0;

		/**
		 * @brief Get a vector of handles to all plugins.
		 * @return Vector of plugin handles.
		 */
		virtual std::vector<PluginHandle> GetPlugins() const = 0;
	};
} // namespace plugify
