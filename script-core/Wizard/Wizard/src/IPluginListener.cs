namespace Wizard
{
	public interface IPluginListener
	{
		/**
		 * @brief Called when a plugin is loaded.
		 *
		 * @param id		Id of the plugin.
		 */
		void OnLoaded(ulong id);

		/**
		 * @brief Called when a plugin is unloaded.
		 *
		 * @param id		Id of the plugin.
		 */
		void OnUnloaded(ulong id);

		/**
		 * @brief Called when a plugin is paused.
		 *
		 * @param id		Id of the plugin.
		 */
		void OnPaused(ulong id);

		/**
		 * @brief Called when a plugin is unpaused.
		 *
		 * @param id		Id of the plugin.
		 */
		void OnUnpaused(ulong id);

		/**
		 * Called after all plugins have been loaded. This is called once for every plugin. If a plugin late loads, it will be called immediately after OnStart().
		 */
		void OnAllLoaded();
	}
}