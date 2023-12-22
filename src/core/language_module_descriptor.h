#pragma once

namespace wizard {
    /**
     * Descriptor for language module. Contains all the information present in an wmodule file.
     */
    struct LanguageModuleDescriptor {
        /** Name of this module */
        std::string name;

        // TODO: Add more
    };

    /**
     * A Module reference descriptor that includes the necessary information to find a module for a specific plugin.
     */
    struct LanguageModuleInfo {
        /** Name of the module */
        std::string name;
    };
}