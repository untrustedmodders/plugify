# Plugify Project Documentation

## Plugin Configuration
Each plugin within the Plugify ecosystem should have an associated configuration file named .pplugin. 
This configuration file contains essential information that both the core and language modules 
use during the plugin loading process.

## .pplugin File Structure
The .pplugin file is a json configuration file with a simple key-value structure. It provides crucial details about the plugin, allowing seamless integration into the Plugify framework.

Example .pplugin file:
```json
{
  "fileVersion": 1,
  "version": 1,
  "versionName": "1.0",
  "friendlyName": "Sample Plugin",
  "description": "That a example plugin",
  "createdBy": "untrustedmodders",
  "createdByURL": "https://github.com/untrustedmodders/",
  "docsURL": "https://github.com/untrustedmodders/sample_plugin",
  "downloadURL": "https://github.com/untrustedmodders/sample_plugin/releases/download/v1.0/sample_plugin.zip",
  "updateURL": "https://raw.githubusercontent.com/untrustedmodders/sample_plugin/main/sample_plugin.json",
  "entryPoint": "bin/sample_plugin",
  "supportedPlatforms": [],
  "languageModule": {
    "name": "cpp"
  },
  "dependencies": [
    {
      "name": "dynohook"
    }
  ],
  "exportedMethods": []
}
```

## Configuration Parameters
- **fileVersion:** The version number of the configuration file format.
- **version:** The version number of the plugin.
- **versionName:** A human-readable version name, such as "1.0".
- **friendlyName:** A user-friendly name for the plugin.
- **description:** A brief description or overview of the plugin. (Currently empty in this example.)
- **createdBy:** The creator or author of the plugin.
- **createdByURL:** The URL linking to the creator's profile or information.
- **docsURL:** The URL linking to the documentation for the plugin.
- **downloadURL:** The URL for downloading the plugin, typically a release package or ZIP file.
- **updateURL:** The URL for checking and fetching updates for the plugin.
- **supportedPlatforms:** An array listing the platforms supported by the plugin. (Currently empty in this example.)
- **entryPoint:** The entry point or main executable for the plugin, specified as "bin/sample_plugin". (Depends on language module)
- **languageModule:** Information about the programming language module used. In this case, it's specified as "cpp" (C++).
- **dependencies:** A list of plugin references specifying the dependencies required for the plugin. This field is crucial for Topological Sorting to load plugins in the correct order of initialization.
- **exportedMethods:** An array describing functions/methods exposed by the plugin. [Read more here.](/basic-types.md])

## Integration with Core and Language Modules
Upon loading a plugin, both the Plugify core and language modules use the information from the .pplugin configuration file. The core relies on the entry point and version to initiate and manage the plugin, while language modules may use additional parameters based on their specific requirements.
Ensure that each plugin's .pplugin file is accurately configured to guarantee a smooth integration process within the Plugify ecosystem.

## Language Module Restriction
Each plugin should specify the programming language it is written in. The language parameter in the .pplugin file corresponds to the type of language module used by the plugin. It is important to note that multiple language modules with the same language are not allowed. This ensures a clear and unambiguous association between plugins and their respective language modules.

## Dependency Management
The information provided in the dependencies field is used for dependency management. The Plugify core utilizes Topological Sorting to determine the correct order for loading plugins based on their dependencies. This ensures that plugins with dependencies are initialized in the appropriate sequence, avoiding potential initialization issues.

Here's the representation of dependency in JSON format along with descriptions for each field:

```json
{
    "name": "dynohook",
    "optional": false,
    "supportedPlatforms": ["windows", "linux"],
    "requestedVersion": 2
}
```

- **name:** The name of the plugin reference. This field identifies the dependency by its unique name within the Plugify ecosystem.
- **optional:** Indicates whether the plugin reference is optional. If set to true, the core system will consider the dependency as optional, and its absence won't prevent the plugin from loading. If set to false, the dependency is mandatory for the plugin to function correctly.
- **supportedPlatforms:** Specifies the platforms supported by the plugin reference. This field helps ensure that the dependency is compatible with the current platform during initialization.
- **requestedVersion:** An optional field representing the requested version of the plugin reference. If provided, the core system will validate that dependency matches the specified version. If not provided, any compatible version may be used.