#pragma once

#include <memory>
#include <string>
#include <vector>

#include "plugify/mem_addr.hpp"
#include "plugify/types.hpp"
#include "plugify/method.hpp"

namespace plugify {
	class Extension;
	class Provider;

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
		 * @param provider Weak ptr to the Plugify provider.
		 * @param module Ref to the language module being initialized.
		 * @return Result of the initialization, either InitResultData or string.
		 */
		virtual Result<InitData> Initialize(const Provider& provider, const Extension& module) = 0;

		/**
		 * @brief Shutdown the language module.
		 */
		virtual void Shutdown() = 0;

		/**
		 * @brief Handle actions to be performed on each frame.
		 * @param deltaTime The time delta since the last update.
		 */
		virtual void OnUpdate(Duration deltaTime) = 0;

        /**
	     * @brief
	     */
        //virtual void OnPluginInitialize(const Extension& plugin) = 0;

		/**
		 * @brief Handle plugin load event.
		 * @param plugin Ref to the loaded plugin.
		 * @return Result of the load event, either LoadResultData or string.
		 */
		virtual Result<LoadData> OnPluginLoad(const Extension& plugin) = 0;

		/**
		 * @brief Handle plugin start event.
		 * @param plugin Ref to the started plugin.
		 */
		virtual void OnPluginStart(const Extension& plugin) = 0;

		/**
		 * @brief Handle plugin update event.
		 * @param plugin Ref to the started plugin.
		 * @param deltaTime The time delta since the last update.
		 */
		virtual void OnPluginUpdate(const Extension& plugin, Duration deltaTime) = 0;

		/**
		 * @brief Handle plugin end event.
		 * @param plugin Ref to the ended plugin.
		 */
		virtual void OnPluginEnd(const Extension& plugin) = 0;

		/**
		 * @brief Handle method export event.
		 * @param plugin Ref to the plugin exporting a method.
		 */
		virtual void OnMethodExport(const Extension& plugin) = 0;

		/**
		* @brief Determine if language module is build with debugging mode.
		* @return True if the assembly is build with debugging, false otherwise.
		*/
		virtual bool IsDebugBuild() = 0;
	};
} // namespace plugify
