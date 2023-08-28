using System;
        
#nullable enable

namespace Wizard
{
    public class Plugin : IEquatable<Plugin>
    {
        public enum Result
        {
            Continue, // Continue with the original action
            Changed,  // Inputs or outputs have been overridden with new values
            Handled,  // Handle the action at the end (don't call it)
            Stop      // Immediately stop the hook chain and handle the original
        }

        protected Plugin()
        {
            Id = ulong.MaxValue;
        }

        internal Plugin(ulong id)
        {
            Id = id;
        }

        public readonly ulong Id;

        public Plugin? FindPluginByName(string name)
        {
            ulong pluginId = InternalCalls.Plugin_FindPluginByName(name);
            if (pluginId == ulong.MaxValue)
                return null;

            return new Plugin(pluginId);
        }

        public static bool operator ==(Plugin plugin1, Plugin plugin2)
        {
            return plugin1.Id == plugin2.Id;
        }

        public static bool operator !=(Plugin plugin1, Plugin plugin2)
        {
            return plugin1.Id != plugin2.Id;
        }

        public bool Equals(Plugin? otherPlugin)
        {
            return !ReferenceEquals(otherPlugin, null) && Id == otherPlugin.Id;
        }

        public override bool Equals(object? obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            if (ReferenceEquals(this, obj)) return true;
            return obj.GetType() == GetType() && Equals((Plugin)obj);
        }
        
        public bool IsNull()
        {
            return Id == ulong.MaxValue;
        }

        public override int GetHashCode()
        {
            return (int)Id;
        }
    }
}

#nullable disable