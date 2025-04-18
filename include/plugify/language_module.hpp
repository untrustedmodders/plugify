#pragma once

#include <array>
#include <cstring>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "method.hpp"
#include "mem_addr.hpp"
#include "date_time.hpp"

namespace plugify {
	class PluginHandle;
	class ModuleHandle;
	class MethodHandle;
	class IPlugifyProvider;

	/**
	 * @struct ErrorData
	 * @brief Holds information about an error.
	 *
	 * The ErrorData structure contains a string describing an error.
	 */
	struct ErrorData {
		std::vector<char> error; ///< Description of the error.

		ErrorData(std::string_view str) {
			error.assign(str.begin(), str.end());
			if (error.back() != 0) {
				error.emplace_back('\0');
			}
		}
	};

	/**
	 * @struct InitResultData
	 * @brief Holds information about the initialization result.
	 *
	 * The InitResultData structure is used to represent the result of a language module initialization.
	 */
	struct InitResultData {
		MethodTable table;
	};

	/**
	 * @struct LoadResultData
	 * @brief Holds information about the load result.
	 *
	 * The LoadResultData structure is used to represent the result of loading a plugin in a language module.
	 */
	struct LoadResultData {
		std::vector<MethodData> methods; ///< Methods exported by the loaded plugin.
		MemAddr data; /// User data.
		MethodTable table;
	};

	/**
	 * @typedef InitResult
	 * @brief Represents the result of language module initialization.
	 *
	 * The InitResult is a variant that can hold either InitResultData or ErrorData.
	 */
	using InitResult = std::variant<InitResultData, ErrorData>;

	/**
	 * @typedef LoadResult
	 * @brief Represents the result of loading a plugin in a language module.
	 *
	 * The LoadResult is a variant that can hold either LoadResultData or ErrorData.
	 */
	using LoadResult = std::variant<LoadResultData, ErrorData>;

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
		 * @param provider Weak pointer to the Plugify provider.
		 * @param module Handle to the language module being initialized.
		 * @return Result of the initialization, either InitResultData or ErrorData.
		 */
		virtual InitResult Initialize(std::weak_ptr<IPlugifyProvider> provider, ModuleHandle module) = 0;

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
		virtual LoadResult OnPluginLoad(PluginHandle plugin) = 0;

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
