namespace Wizard
{
	public enum Result
	{
		Continue, // Continue with the original action
		Changed,  // Inputs or outputs have been overridden with new values
		Handled,  // Handle the action at the end (don't call it)
		Stop      // Immediately stop the hook chain and handle the original
	}
}