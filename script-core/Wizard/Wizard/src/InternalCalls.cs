using System.Runtime.CompilerServices;

namespace Wizard
{
	public static class InternalCalls
	{
		#region Plugin
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal static extern ulong Plugin_FindPluginByName(string name);
		#endregion
	}
}
