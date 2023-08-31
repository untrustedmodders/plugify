using System;
using Wizard;

namespace Plugin1
{
	[PluginInfo(Name, Description, Author, Version, URL, new []{"SamplePlugin2", "Plugin3.SamplePlugin"})]
    public class SamplePlugin : Plugin
    {
	    private const string Name = "SamplePlugin1";
	    private const string Description = "Sample plugin to help developers";
	    private const string URL = "http://www.site.net/";
	    private const string Author = "qubka";
	    private const string Version = "0.0.0.0";

		/**
		 * Called when the plugin is fully initialized and all known external references are resolved.
		 * This is only called once in the lifetime of the plugin, and is paired with OnDestroy().
		 */
		void OnCreate()
		{
			Console.Write(Name + ": OnCreate\n");
			Console.Write($"FindPluginByName: SamplePlugin2 => (Found: id) => {FindPluginByName("SamplePlugin2")?.Id}\n");
		}

		/**
		 * Called when the plugin is about to be released.
		 */
		void OnDestroy()
		{
			Console.Write(Name + ": OnDestroy\n");
		}
    }
}