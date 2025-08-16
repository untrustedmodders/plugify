#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <plugify/api/handle.hpp>
#include <plugify/api/path.hpp>
#include <plugify/api/version.hpp>
#include <plugify/asm/assembly.hpp>

#include <plugify_export.h>

namespace plugify {
	class Provider;
	class ModuleHandle;
	class PluginHandle;
	enum class Severity;

	/**
	 * @class ProviderHandle
	 * @brief Wrapper handle for the Provider, which is provided to the user and implemented in the core.
	 */
	class PLUGIFY_API ProviderHandle : public Handle<const Provider> {
		using Handle::Handle;
	public:
		/**
		 * @brief Log a message with a specified severity level.
		 * @param msg The message to log.
		 * @param severity The severity level of the log message.
		 */
		void Log(std::string_view msg, Severity severity) const;

		/**
		 * @brief Get the base directory of the Plugify system.
		 * @return Reference to the base directory path.
		 */
		std::filesystem::path_view GetBaseDir() const noexcept;

		/**
		 * @brief Get the configuration directory of the Plugify system.
		 * @return Reference to the configuration directory path.
		 */
		std::filesystem::path_view GetConfigsDir() const noexcept;

		/**
		 * @brief Get the data directory of the Plugify system.
		 * @return Reference to the data directory path.
		 */
		std::filesystem::path_view GetDataDir() const noexcept;

		/**
		 * @brief Get the logs directory of the Plugify system.
		 * @return Reference to the logs directory path.
		 */
		std::filesystem::path_view GetLogsDir() const noexcept;

		/**
		 * @brief Checks if the preference for using own symbols is enabled.
		 *
		 * This function returns whether the preference for loading symbols from
		 * the library itself (as opposed to shared libraries) is enabled. This
		 * can be useful in environments where symbol conflicts might arise
		 * and isolating symbols to their respective libraries is necessary.
		 *
		 * @note Start using RTLD_DEEPBIND flag by default.
		 * 
		 * @return True if the preference for using own symbols is enabled, false otherwise.
		 */
		bool IsPreferOwnSymbols() const noexcept;
		
		/**
		 * @brief Checks if a plugin with the specified name is loaded.
		 * 
		 * @param name The name of the plugin to check.
		 * @param constraint Optional required constraint of the plugin.
		 * @return True if the plugin is loaded and meets the version requirements, false otherwise.
		 */
		bool IsPluginLoaded(std::string_view name, std::optional<Constraint> constraint = {}) const noexcept;
		
		/**
		 * @brief Checks if a language module with the specified name is loaded.
		 * 
		 * @param name The name of the language module to check.
		 * @param constraint Optional required constraint of the language module.
		 * @return True if the language module is loaded and meets the version requirements, false otherwise.
		 */
		bool IsModuleLoaded(std::string_view name, std::optional<Constraint> constraint = {}) const noexcept;

		/**
		 * @brief Finds a plugin by its name.
		 *
		 * This function attempts to find a plugin with the specified name in the Plugify system.
		 * If a plugin with the given name is found, a handle to it is returned. Otherwise,
		 * an empty handle is returned.
		 *
		 * @param name The name of the plugin to find.
		 * @return A handle to the plugin if found, or an empty handle if not found.
		 */
		PluginHandle FindPlugin(std::string_view name) const noexcept;

		/**
		 * @brief Finds a language module by its name.
		 *
		 * This function attempts to find a language module with the specified name in the Plugify system.
		 * If a module with the given name is found, a handle to it is returned. Otherwise,
		 * an empty handle value is returned.
		 *
		 * @param name The name of the language module to find.
		 * @return A handle to the module if found, or an empty handle if not found.
		 */
		ModuleHandle FindModule(std::string_view name) const noexcept;
	};
} // namespace plugify
