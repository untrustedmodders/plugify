namespace Wizard 
{
	public interface IServerListener
	{
		/**
		 * Called when the map has loaded, servercfgfile (server.cfg) has been executed, and all plugin configs are done executing. This is the best place to initialize plugin functions which are based on cvar data.
		 */
		void OnConfigsExecuted();

		/**
		 * Called once per server frame. Server performance is very sensitive to the execution time of this function so keep anything you do in this function to a minimum.
		 */
		//void OnGameFrame(bool simulating); -> OnUpdate
		
		/**
		 * Called when a server is changing to a new level or is being shutdown. Remove any map specific allocations in this call. 
		 */
		void OnLevelShutdown();	
		
		/**
		 * Called on level (map) startup, it is the first function called as a server enters a new level.
		 */
		bool OnLevelInit(string mapName);	
		
		/**
		 * This is called when the server successfully enters a new map, this will happen after the OnLevelInit call.
		 */
		void OnLevelStart();	
		
		/**
		 * Called when an entity is created.
		 */
		void OnEntityCreated(Entity entity, string className);
		
		/**
		 * Called when an entity is destroyed.
		 */
		void OnEntityDestroyed(Entity entity, string className);
	}
}
		