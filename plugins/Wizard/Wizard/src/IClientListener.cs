namespace Wizard 
{
	public interface IClientListener
	{
		/**
		 * Called when a client receives an auth ID.
		 */
		void OnClientAuthorized(Entity entity, string authId);

		/**
		 * Called when a client is sending a command.
		 */
		Result OnClientCommand(Entity entity, string[] args);

		/**
		 * Called on client connection. If you return true, the client will be allowed in the server. If you return false (or return nothing), the client will be rejected. If the client is rejected by this forward or any other, OnClientDisconnect will not be called.
		 */
		bool OnClientConnect(Entity entity, string name, string address, ref string reject);
		
		/**
		 * Called once a client successfully connects. This callback is paired with OnClientDisconnect.
		 **/
		void OnClientConnected(Entity entity);

		/**
		 * Called when a client is disconnecting from the server.
		 */
		void OnClientDisconnect(Entity entity);

		/**
		 * Called when a client is fully disconnected from the server.
		 */
		void OnClientDisconnected(Entity entity);

		/**
		 * Called when a client spawns into a server. This is called before the server spawn function.
		 */
		void OnClientPutInServer(Entity entity, string playerName);

		/**
		 * Called after a client is fully spawned. Use this call to change any player specific settings.
		 */
		void OnClientActive(Entity entity, bool load);

		/**
		 * Called when player specific cvars about a player change (for example the users name). Use this function to control what operations a user is allowed to perform to their settings (for example limiting usernames).
		 */
		void OnClientSettingsChanged(Entity entity);
	}
}
