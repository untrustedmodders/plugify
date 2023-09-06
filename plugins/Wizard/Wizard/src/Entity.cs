using System;

#nullable enable

namespace Wizard
{
	public struct Entity : IEquatable<Entity>, IComparable<Entity>
	{
		public readonly ulong Ptr;

		public Entity(ulong ptr)
		{
			Ptr = ptr;
		}

		public static bool operator ==(Entity lhs, Entity rhs)
		{
			return lhs.Ptr == rhs.Ptr;
		}

		public static bool operator !=(Entity lhs, Entity rhs)
		{
			return lhs.Ptr != rhs.Ptr;
		}
		
		public int CompareTo(Entity other)
        {
            return Ptr.CompareTo(other.Ptr);
        }
		
		public bool Equals(Entity other)
        {
            return Ptr == other.Ptr;
        }

		public override bool Equals(object? compare)
        {
            return compare is Entity compareEntity && Ptr == compareEntity.Ptr;
        }
        
        public bool IsNull()
        {
            return Ptr == 0;
        }

        public override int GetHashCode()
        {
            return Ptr.GetHashCode();
        }
		
		public static Entity Null => new(0);
		
		public override string ToString()
        {
            return Equals(Null) ? "Entity.Null" : $"Entity(Ox{Ptr})";
        }
	}
}

#nullable disable