#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>
#include <plugify/assembly.h>
#include <plugify/reference_wrapper.h>
#include <plugify_export.h>

namespace plugify {
	class PlugifyProvider;
	class PlugifyProvider;
	enum class Severity : uint8_t;

	/**
 	 * @brief Interface class for the PlugifyProvider, which is provided to the user and implemented in the core.
	 *        The PlugifyProvider is responsible for managing and providing essential functionality to the Plugify system.
	 */
	class PLUGIFY_API IPlugifyProvider : public Ref<PlugifyProvider> {
		using Ref::Ref;
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
		[[nodiscard]] const std::filesystem::path& GetBaseDir() const noexcept;

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
		[[nodiscard]] bool IsPreferOwnSymbols() const noexcept;
		
		/**
		 * @brief Checks if a plugin with the specified name is loaded.
		 * 
		 * This function checks if a plugin with the specified name is currently loaded.
		 * Optionally, you can specify a required version and set the `minimum` parameter
		 * to true to check if the loaded version meets or exceeds the required version.
		 * 
		 * @param name The name of the plugin to check.
		 * @param requiredVersion Optional required version of the plugin.
		 * @param minimum If true, checks if the loaded version meets or exceeds the required version.
		 * @return True if the plugin is loaded and meets the version requirements, false otherwise.
		 */
		[[nodiscard]] bool IsPluginLoaded(std::string_view name, std::optional<int32_t> requiredVersion = {}, bool minimum = false) const noexcept;
		
		/**
		 * @brief Checks if a language module with the specified name is loaded.
		 * 
		 * This function checks if a language module with the specified name is currently loaded.
		 * Optionally, you can specify a required version and set the `minimum` parameter
		 * to true to check if the loaded version meets or exceeds the required version.
		 * 
		 * @param name The name of the language module to check.
		 * @param requiredVersion Optional required version of the language module.
		 * @param minimum If true, checks if the loaded version meets or exceeds the required version.
		 * @return True if the language module is loaded and meets the version requirements, false otherwise.
		 */
		[[nodiscard]] bool IsModuleLoaded(std::string_view name, std::optional<int32_t> requiredVersion = {}, bool minimum = false) const noexcept;
	};
	static_assert(is_ref_v<IPlugifyProvider>);
} // namespace plugify