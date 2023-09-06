using System;

namespace Wizard
{
    [AttributeUsage(AttributeTargets.Class)]
    public class PluginInfo : Attribute
    {
        /** @brief A unique name for the plugin */
        private string _name;

        /** @brief A user friendly description of the plugin */
        private string _description;

        /** @brief An author of this plugin */
        private string _author;

        /** @brief A version string for this plugin */
        private string _version;
        
        /** @brief A url link to this plugin */
        private string _url;

        /** @brief A dependency plugins list of this plugin */
        private string[] _dependencies;

        public PluginInfo(string name, string description, string author, string version, string url)
        {
            _name = name;
            _description = description;
            _author = author;
            _version = version;
            _url = url;
            _dependencies = new[] { "" };
        }
        
        public PluginInfo(string name, string description, string author, string version, string url, string[] dependencies)
        {
            _name = name;
            _description = description;
            _author = author;
            _version = version;
            _url = url;
            _dependencies = dependencies.Length > 0 ? dependencies : new[] { "" };
        }
    }
}
