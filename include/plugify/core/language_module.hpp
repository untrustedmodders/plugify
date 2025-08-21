#pragma once

#include <memory>
#include <string>
#include <vector>

#include "plugify/_/method_handle.hpp"
#include "date_time.hpp"
#include "plg/expected.hpp"
#include "plugify/asm/mem_addr.hpp"

namespace plugify {
	class PluginHandle;
	class ModuleHandle;
	class MethodHandle;
	class ProviderHandle;

	/**
	 * @struct InitData
	 * @brief Holds information about the initialization result.
	 *
	 * The InitResult structure is used to represent the result of a language module initialization.
	 */
	struct InitData {
		MethodTable table;
	};

	/**
	 * @struct LoadData
	 * @brief Holds information about the load result.
	 *
	 * The LoadResult structure is used to represent the result of loading a plugin in a language module.
	 */
	struct LoadData {
		std::vector<MethodData> methods; ///< Methods exported by the loaded plugin.
		MemAddr data; ///< Data associated with the loaded plugin.
		MethodTable table; ///< Method table for the loaded plugin.
	};

	template<typename T>
	using Result = plg::expected<T, std::string>;

	/**
	 * @class ILanguageModule
	 * @brief Interface for user-implemented language modules.
	 *
	 * The ILanguageModule interface defines methods that should be implemented by user-written language modules.
	 */
	class ILanguageModule {
	protected:
		~ILanguageModule() = default;

	public:
		/**
		 * @brief Initialize the language module.
		 * @param provider Handle to the Plugify provider.
		 * @param module Handle to the language module being initialized.
		 * @return Result of the initialization, either InitResultData or ErrorData.
		 */
		virtual Result<InitData> Initialize(ProviderHandle provider, ModuleHandle module) = 0;

		/**
		 * @brief Shutdown the language module.
		 */
		virtual void Shutdown() = 0;

		/**
		 * @brief Handle actions to be performed on each frame.
		 * @param dt The time delta since the last update.
		 */
		virtual void OnUpdate(DateTime dt) = 0;

		/**
		 * @brief Handle plugin load event.
		 * @param plugin Handle to the loaded plugin.
		 * @return Result of the load event, either LoadResultData or ErrorData.
		 */
		virtual Result<LoadData> OnPluginLoad(PluginHandle plugin) = 0;

		/**
		 * @brief Handle plugin start event.
		 * @param plugin Handle to the started plugin.
		 */
		virtual void OnPluginStart(PluginHandle plugin) = 0;

		/**
		 * @brief Handle plugin update event.
		 * @param plugin Handle to the started plugin.
		 * @param dt The time delta since the last update.
		 */
		virtual void OnPluginUpdate(PluginHandle plugin, DateTime dt) = 0;

		/**
		 * @brief Handle plugin end event.
		 * @param plugin Handle to the ended plugin.
		 */
		virtual void OnPluginEnd(PluginHandle plugin) = 0;

		/**
		 * @brief Handle method export event.
		 * @param plugin Handle to the plugin exporting a method.
		 */
		virtual void OnMethodExport(PluginHandle plugin) = 0;

		/**
		* @brief Determine if language module is build with debugging mode.
		* @return True if the assembly is build with debugging, false otherwise.
		*/
		virtual bool IsDebugBuild() = 0;
	};
} // namespace plugify
