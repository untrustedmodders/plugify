#pragma once

#include <array>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <cstring>
#include <plugify/mem_addr.h>

namespace plugify {
	class PluginRef;
	class ModuleRef;
	class MethodRef;
	class IPlugifyProvider;

	/**
	 * @typedef MethodData
	 * @brief Represents data related to a plugin method.
	 *
	 * The MethodData type is a pair consisting of a method reference and a
	 * pointer to the method's address (void*).
	 */
	using MethodData = std::pair<MethodRef, MemAddr>;

	/**
	 * @struct ErrorData
	 * @brief Holds information about an error.
	 *
	 * The ErrorData structure contains a string describing an error.
	 */
	struct ErrorData {
		//string error; ///< Description of the error.

		std::array<char, 256> error{}; ///< Description of the error.

		ErrorData(std::string_view str) {
			std::memcpy(error.data(), str.data(), std::min(str.length(), error.size() - 1));
		}
	};

	/**
	 * @struct InitResultData
	 * @brief Holds information about the initialization result.
	 *
	 * The InitResultData structure is used to represent the result of a language module initialization.
	 */
	struct InitResultData {};

	/**
	 * @struct LoadResultData
	 * @brief Holds information about the load result.
	 *
	 * The LoadResultData structure is used to represent the result of loading a plugin in a language module.
	 */
	struct LoadResultData {
		std::vector<MethodData> methods; ///< Methods exported by the loaded plugin.
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
		 * @param module Reference to the language module being initialized.
		 * @return Result of the initialization, either InitResultData or ErrorData.
		 */
		virtual InitResult Initialize(std::weak_ptr<IPlugifyProvider> provider, ModuleRef module) = 0;

		/**
		 * @brief Shutdown the language module.
		 */
		virtual void Shutdown() = 0;

		/**
		 * @brief Handle plugin load event.
		 * @param plugin Reference to the loaded plugin.
		 * @return Result of the load event, either LoadResultData or ErrorData.
		 */
		virtual LoadResult OnPluginLoad(PluginRef plugin) = 0;

		/**
		 * @brief Handle plugin start event.
		 * @param plugin Reference to the started plugin.
		 */
		virtual void OnPluginStart(PluginRef plugin) = 0;

		/**
		 * @brief Handle plugin end event.
		 * @param plugin Reference to the ended plugin.
		 */
		virtual void OnPluginEnd(PluginRef plugin) = 0;

		/**
		 * @brief Handle method export event.
		 * @param plugin Reference to the plugin exporting a method.
		 */
		virtual void OnMethodExport(PluginRef plugin) = 0;

		/**
		* @brief Determine if language module is build with debugging mode.
		* @return True if the assembly is build with debugging, false otherwise.
		*/
		virtual bool IsDebugBuild() = 0;
	};
} // namespace plugify
