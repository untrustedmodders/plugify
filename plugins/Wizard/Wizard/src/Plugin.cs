using System;

#nullable enable

namespace Wizard
{
    public class Plugin : IEquatable<Plugin>, IComparable<Plugin>
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
        
        public int CompareTo(Plugin? other)
        {
            if (ReferenceEquals(this, other)) return 0;
            if (ReferenceEquals(null, other)) return 1;
            return Id.CompareTo(other.Id);
        }
        
        public bool Equals(Plugin? other)
        {
            return !ReferenceEquals(other, null) && Id == other.Id;
        }

        public override bool Equals(object? obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            if (ReferenceEquals(this, obj)) return true;
            return obj.GetType() == GetType() && Id == ((Plugin)obj).Id;
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
            return IsNull() ? "Plugin.Null" : $"Plugin({Id})";
        }
    }
}

#nullable disable