using System;

#nullable enable

namespace Wizard
{
    public class Plugin : IEquatable<Plugin>
    {
        public readonly ulong Id;

        protected Plugin()
        {
            Id = ulong.MaxValue;
        }

        internal Plugin(ulong id)
        {
            Id = id;
        }

        public Plugin? FindPluginByName(string name)
        {
            return (Plugin?) InternalCalls.Plugin_FindPluginByName(name);
        }

        public static bool operator ==(Plugin lhs, Plugin rhs)
        {
            return lhs.Id == rhs.Id;
        }

        public static bool operator !=(Plugin lhs, Plugin rhs)
        {
            return lhs.Id != rhs.Id;
        }

        public bool Equals(Plugin? other)
        {
            return !ReferenceEquals(other, null) && Id == other.Id;
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
            return Id.GetHashCode();
        }

		public override string ToString()
        {
            return Equals(null) ? "Plugin.Null" : $"Plugin({Id})";
        }
    }
}

#nullable disable