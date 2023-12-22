#pragma once

#include "plugin_descriptor.h"

namespace wizard {
    /**
     * Descriptor for plugins. Contains all the information contained within a wpluginmanifest file.
     */
    struct PluginManifestEntry {
        // TODO:
    };

    /**
     * Manifest of plugins. Descriptor for plugins. Contains all the information contained within a plugins.json file.
     */
    struct PluginManifest {
        /** List of plugins in this manifest */
        std::vector<PluginManifestEntry> contents;
    };
}