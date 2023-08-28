using System;
using Wizard;

namespace Plugin1
{
    public class SamplePlugin : Plugin, IPluginInfo
    {
#region Plugin state

		/**
		 * Called before OnStart, in case the plugin wants to check for load failure. 
		 */
		bool OnLoad()
		{
			
			Console.Write(GetName() + ": OnLoad\n");
			return true;
		}

		/**
		 * Called when a plugin is unloaded, use this to disable any asynchronous tasks and remove any callbacks you have registered with the engine (for example a game events listener). Additionally, this function will be called if false is returned by Load().
		 */
		void OnUnload()
		{
			Console.Write(GetName() + ": OnUnload\n");
		}

		/**
		 * Called when the plugin is fully initialized and all known external references are resolved.
		 * This is only called once in the lifetime of the plugin, and is paired with OnEnd().
		 */
		void OnStart()
		{
			Console.Write(GetName() + ": OnStart\n");
		}

		/**
		 * Called when the plugin is about to be unloaded.
		 */
		void OnEnd()
		{
			Console.Write(GetName() + ": OnEnd\n");
		}

		/**
		 * Called when the operation of the plugin is paused (i.e it will stop receiving callbacks but should not be unloaded).
		 */
		void OnPause()
		{
			Console.Write(GetName() + ": OnPause\n");
		}

		/**
		 * Called when a plugin is brought out of the paused state. You should re-enable any asynchronous events your plugin uses in this call
		 */
		void OnUnpause()
		{
			Console.Write(GetName() + ": OnUnpause\n");
		}

#endregion

// IPluginInfo
		public string GetName() { return "Sample Plugin 1"; }
		public string GetDescription() { return "Sample plugin to help developers"; }
		public string GetURL() { return "http://www.site.net/"; }
		public string GetTag() { return "SAMPLE"; }
		public string GetLicence() { return "SAMPLE"; }
		public string GetAuthor() { return "qubka"; }
		public string GetVersion() { return "0.0.0.0"; }
		public string GetDate() { return "01.01.1970"; }
    }
}