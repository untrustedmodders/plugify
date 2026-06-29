#pragma once

#include <memory>
#include <string>
#include <vector>
#include <chrono>

#include "plugify/address.hpp"
#include "plugify/method.hpp"
#include "plugify/types.hpp"

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
	 * The LoadResult structure is used to represent the result of loading a plugin in a language
	 * module.
	 */
	struct LoadData {
		std::vector<MethodData> methods;  ///< Methods exported by the loaded plugin.
		Address data;                     ///< Data associated with the loaded plugin.
		MethodTable table;                ///< Method table for the loaded plugin.
	};

	/**
	 * @class ILanguageModule
	 * @brief Interface for user-implemented language modules.
	 *
	 * The ILanguageModule interface defines methods that should be implemented
	 * by user-written language modules.
	 */
	class ILanguageModule {
	protected:
		~ILanguageModule() = default;

	public:
		/**
		 * @brief Initialize the language module.
		 * @param provider Weak ptr to the Plugify provider.
		 * @param module Ref to the language module being initialized.
		 * @return Result of the initialization, either InitData or an error string.
		 */
		virtual Result<InitData> Initialize(const Provider& provider, const Extension& module) = 0;

		/**
		 * @brief Shutdown the language module.
		 * @return Result of the shutdown, either void or an error string.
		 */
		virtual Result<void> Shutdown() = 0;

		/**
		 * @brief Handle actions to be performed on each frame.
		 * @param deltaTime The time delta since the last update.
		 * @return Result of the update, either void or an error string.
		 */
		virtual Result<void> OnUpdate(std::chrono::milliseconds deltaTime) = 0;

		/**
		 * @brief Handle plugin load event.
		 * @param plugin Ref to the plugin being loaded.
		 * @return Result of the load event, either LoadData or an error string.
		 */
		virtual Result<LoadData> OnPluginLoad(const Extension& plugin) = 0;

		/**
		 * @brief Handle plugin start event.
		 * @param plugin Ref to the plugin being started.
		 * @return Result of the start, either void or an error string.
		 */
		virtual Result<void> OnPluginStart(const Extension& plugin) = 0;

		/**
		 * @brief Handle plugin update event.
		 * @param plugin Ref to the plugin being updated.
		 * @param deltaTime The time delta since the last update.
		 * @return Result of the update, either void or an error string.
		 */
		virtual Result<void> OnPluginUpdate(const Extension& plugin, std::chrono::milliseconds deltaTime) = 0;

		/**
		 * @brief Handle plugin end event.
		 * @param plugin Ref to the plugin being ended.
		 * @return Result of the end, either void or an error string.
		 */
		virtual Result<void> OnPluginEnd(const Extension& plugin) = 0;

		/**
		 * @brief Handle method export event.
		 * @param plugin Ref to the plugin exporting a method.
		 * @return Result of the export, either void or an error string.
		 */
		virtual Result<void> OnMethodExport(const Extension& plugin) = 0;

		/**
		 * @brief Determine if the language module was built in debug mode.
		 * @return True if built with debugging enabled, false otherwise.
		 */
		virtual bool IsDebugBuild() const noexcept = 0;
	};
}  // namespace plugify
