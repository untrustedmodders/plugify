#pragma once

#include <chrono>

#include "plugify/types.hpp"

namespace plugify {
	class Extension;

	/**
	 * @class IExtensionLifecycle
	 * @brief Interface for observing what happens to every extension.
	 *
	 * An implementation is called after each lifecycle step of a plugin or a
	 * language module has been carried out, which makes it the place for hooks a
	 * host application wants around loading without touching the pipeline itself.
	 * The listener is optional: when none is registered, nothing is called.
	 *
	 * The callbacks observe, they do not decide. Whether a step happened at all
	 * is up to the pipeline, and the state of the extension passed in says how it
	 * went.
	 *
	 * @see PlugifyBuilder::WithExtensionLifecycle, Extension
	 */
	class IExtensionLifecycle {
	public:
		/**
		 * @brief Virtual destructor.
		 */
		virtual ~IExtensionLifecycle() = default;

		/**
		 * @brief Called after an extension has been loaded.
		 * @param extension The extension that was loaded.
		 */
		virtual void OnLoad(Extension& extension) = 0;

		/**
		 * @brief Called after an extension has been unloaded.
		 * @param extension The extension that was unloaded.
		 * @note Runs after its assembly and its language module have been given back.
		 */
		virtual void OnUnload(Extension& extension) = 0;

		/**
		 * @brief Called after an extension has been started.
		 * @param extension The extension that was started.
		 */
		virtual void OnStart(Extension& extension) = 0;

		/**
		 * @brief Called after an extension has been ended.
		 * @param extension The extension that was ended.
		 */
		virtual void OnEnd(Extension& extension) = 0;

		/**
		 * @brief Called after the update callback of an extension has run.
		 * @param extension The extension that was updated.
		 * @param deltaTime Time since the previous update.
		 * @note Only for extensions that have an update callback at all, see
		 * MethodTable::hasUpdate.
		 */
		virtual void OnUpdate(Extension& extension, std::chrono::milliseconds deltaTime) = 0;

		/**
		 * @brief Called after the methods of an extension have been exported.
		 * @param extension The extension whose methods were exported.
		 */
		virtual void OnExport(Extension& extension) = 0;
	};
}
