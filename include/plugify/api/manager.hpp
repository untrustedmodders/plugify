#pragma once

#include <cstdint>
#include <string>
#include <filesystem>
#include <functional>

#include <plugify/api/handle.hpp>
#include <plugify/api/date_time.hpp>

#include <plugify_export.h>

namespace plugify {
	class Manager;
	class ModuleHandle;
	class PluginHandle;
	class DependencyHandle;

	/**
	 * @brief Represents a unique identifier used for identifying modules, plugins, or other entities.
	 */
	using UniqueId = std::ptrdiff_t;

	/**
	 * @class ManagerHandle
	 * @brief A handle class for an `Manager` class.
	 */
	class PLUGIFY_API ManagerHandle : public Handle<const Manager> {
		using Handle::Handle;
	public:
		/**
		 * @brief Initialize the plugin manager.
		 * @return True if initialization is successful, false otherwise.
		 */
		bool Initialize() noexcept;

		/**
		 * @brief Terminate the plugin manager.
		 */
		void Terminate() noexcept;

		/**
		 * @brief Check if the plugin manager is initialized.
		 * @return True if the plugin manager is initialized, false otherwise.
		 */
		bool IsInitialized() const noexcept;

		/**
		 * @brief Updates the package manager.
		 * @param dt The time delta since the last update.
		 */
		void Update(DateTime dt) noexcept;

		/**
		 * @brief Find a module by name.
		 * @param moduleName Name of the module to find.
		 * @return Handle to the found module.
		 */
		ModuleHandle FindModule(std::string_view moduleName) const noexcept;

		/**
		 * @brief Find a module by unique identifier.
		 * @param moduleId Unique identifier of the module to find.
		 * @return Handle to the found module.
		 */
		ModuleHandle FindModuleFromId(UniqueId moduleId) const noexcept;

		/**
		 * @brief Get a vector of handles to all modules.
		 * @return Vector of module handles.
		 */
		std::vector<ModuleHandle> GetModules() const noexcept;

		/**
		 * @brief Find a plugin by name.
		 * @param pluginName Name of the plugin to find.
		 * @return Handle to the found plugin.
		 */
		PluginHandle FindPlugin(std::string_view pluginName) const noexcept;

		/**
		 * @brief Find a plugin by unique identifier.
		 * @param pluginId Unique identifier of the plugin to find.
		 * @return Handle to the found plugin.
		 */
		PluginHandle FindPluginFromId(UniqueId pluginId) const noexcept;

		/**
		 * @brief Get a vector of handles to all plugins.
		 * @return Vector of plugin handles.
		 */
		std::vector<PluginHandle> GetPlugins() const noexcept;
	};
} // namespace plugify
