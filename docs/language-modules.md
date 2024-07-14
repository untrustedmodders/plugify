# Plugify Project Documentation

## Language Module Configuration
Each language module in Plugify should have a configuration file named .pmodule. 
This file contains essential information that the Plugify core uses to load and manage language modules. 

## .pmodule File Structure
The .pmodule file is a json configuration file with a simple key-value structure. It provides crucial details about the language module, allowing seamless integration into the Plugify framework.

Example .pmodule file:
```json
{
    "fileVersion": 1,
    "version": 1,
    "versionName": "1.0",
    "friendlyName": "C++ language module",
    "language": "cpp",
    "description": "Adds support for C++ plugins",
    "createdBy": "untrustedmodders",
    "createdByURL": "https://github.com/untrustedmodders/",
    "docsURL": "https://github.com/untrustedmodders/cpp-lang-module/README.md",
    "downloadURL": "https://github.com/untrustedmodders/cpp-lang-module/releases/download/v1.0/cpp-lang-module.zip",
    "updateURL": "https://raw.githubusercontent.com/untrustedmodders/cpp-lang-module/main/cpp-lang-module.json",
    "supportedPlatforms": [],
    "forceLoad": false
}
```

## Configuration Parameters
- **fileVersion:** The version number of the configuration file format.
- **version:** The version number of the language module.
- **versionName:** A human-readable version name, such as "1.0".
- **friendlyName:** A user-friendly name for the language module.
- **description:** A brief description or overview of the language module. (Currently empty in this example.)
- **createdBy:** The creator or author of the language module.
- **createdByURL:** The URL linking to the creator's profile or information.
- **docsURL:** The URL linking to the documentation for the language module.
- **downloadURL:** The URL for downloading the language module, typically a release package or ZIP file.
- **updateURL:** The URL for checking and fetching updates for the language module.
- **supportedPlatforms:** An array listing the platforms supported by the language module. (Currently empty in this example.)
- **language:** The identifier for the programming language supported by the module (e.g., "csharp"). This identifier should be unique, and plugins use it to look up the corresponding language module. This uniqueness allows for the seamless swapping of different implementations of the same language module.
- **libraryDirectories:** Optional. Specifies additional directories where the language module can search for libraries.
- **forceLoad:**  Indicates whether the language module should be force-loaded by the Plugify core.

## Purpose 

The Plugify core utilizes the information provided in the .module file to seamlessly integrate and interact with language modules. When the core initializes, it reads the configuration files of registered language modules, allowing for dynamic loading and management of plugins specific to each language.

Language modules are responsible for handling plugin loading, unloading, and execution. The core notifies language modules when plugins are loaded or unloaded, enabling them to manage their respective runtime environments.

By adhering to this configuration file format, language modules can be easily integrated into the Plugify ecosystem, providing a flexible and extensible architecture for supporting various programming languages.

## Creating Language Modules

To extend Plugify with support for additional programming languages, users can create language modules. Language modules are dynamic libraries (shared libraries) that implement the ILanguageModule interface defined in Plugify.

## ILanguageModule Interface

The ILanguageModule interface defines the methods that should be implemented by user-written language modules. Below is an overview of the interface:

```c++
#pragma once

namespace plugify {
    /**
     * @class ILanguageModule
     * @brief Interface for user-implemented language modules.
     *
     * The ILanguageModule interface defines methods that should be implemented by user-written language modules.
     */
    class ILanguageModule {
    protected:
        ~ILanguageModule() = default;

    public:
        /**
         * @brief Initialize the language module.
         * @param provider Weak pointer to the Plugify provider.
         * @param module Reference to the language module being initialized.
         * @return Result of the initialization, either InitResultData or ErrorData.
         */
        virtual InitResult Initialize(std::weak_ptr<IPlugifyProvider> provider, ModuleRef module) = 0;

        /**
         * @brief Shutdown the language module.
         */
        virtual void Shutdown() = 0;

        /**
         * @brief Handle plugin load event.
         * @param plugin Reference to the loaded plugin.
         * @return Result of the load event, either LoadResultData or ErrorData.
         */
        virtual LoadResult OnPluginLoad(PluginRef plugin) = 0;

        /**
         * @brief Handle plugin start event.
         * @param plugin Reference to the started plugin.
         */
        virtual void OnPluginStart(PluginRef plugin) = 0;

        /**
         * @brief Handle plugin end event.
         * @param plugin Reference to the ended plugin.
         */
        virtual void OnPluginEnd(PluginRef plugin) = 0;

        /**
         * @brief Handle method export event.
         * @param plugin Reference to the plugin exporting a method.
         */
        virtual void OnMethodExport(PluginRef plugin) = 0;
    };

} // namespace plugify
```

Follow these steps to create a language module:
- Implement the ILanguageModule interface.
- Initialize variables and systems for managing, loading, starting, and ending plugins for your language.
- Export methods specified in the plugins from the OnPluginLoad, methods are imported during the OnMethodExport.
- Optionally, create function call wrappers using plugify::plugify-function library for dynamic generation of C functions.
- If necessary, use libraries like dyncall to dynamically generate function prototypes and call C functions using their addresses.
- Export an ILanguageModule* GetLanguageModule() method in your library, return an instance of your language module from this method.
