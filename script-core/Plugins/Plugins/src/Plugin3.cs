using System;
using Wizard;

namespace Plugin3
{
    public class SamplePlugin : Plugin, IPluginListener
    {
#region Plugin state

		/**
		 * Called before OnStart, in case the plugin wants to check for load failure. 
		 */
		bool OnLoad()
		{
			Console.Write("SamplePlugin3: OnLoad\n");
			return true;
		}

		/**
		 * Called when a plugin is unloaded, use this to disable any asynchronous tasks and remove any callbacks you have registered with the engine (for example a game events listener). Additionally, this function will be called if false is returned by Load().
		 */
		void OnUnload()
		{
			Console.Write("SamplePlugin3: OnUnload\n");
		}

		/**
		 * Called when the plugin is fully initialized and all known external references are resolved.
		 * This is only called once in the lifetime of the plugin, and is paired with OnEnd().
		 */
		void OnStart()
		{
			Console.Write("SamplePlugin3: OnStart\n");
		}

		/**
		 * Called when the plugin is about to be unloaded.
		 */
		void OnEnd()
		{
			Console.Write("SamplePlugin3: OnEnd\n");
		}

		/**
		 * Called when the operation of the plugin is paused (i.e it will stop receiving callbacks but should not be unloaded).
		 */
		void OnPause()
		{
			Console.Write("SamplePlugin3: OnPause\n");
		}

		/**
		 * Called when a plugin is brought out of the paused state. You should re-enable any asynchronous events your plugin uses in this call
		 */
		void OnUnpause()
		{
			Console.Write("SamplePlugin3: OnUnpause\n");
		}

#endregion

// IPluginListener
		public void OnLoaded(ulong id)
		{
			Console.Write("SamplePlugin3: OnLoaded: " + id  + "\n");
		}

		public void OnUnloaded(ulong id)
		{
			Console.Write("SamplePlugin3: OnUnloaded: " + id  + "\n");
		}

		public void OnPaused(ulong id)
		{
			Console.Write("SamplePlugin3: OnPaused: " + id  + "\n");
		}

		public void OnUnpaused(ulong id)
		{
			Console.Write("SamplePlugin3: OnUnpaused: " + id  + "\n");
		}

		public void OnAllLoaded()
		{
			Console.Write("SamplePlugin3: OnAllLoaded\n");
		}
    }
}