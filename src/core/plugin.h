#pragma once

#include <wizard/plugin.h>
#include "plugin_descriptor.h"

namespace wizard {
    /**
     * Instance of a plugin in memory.
     */
    class Plugin final : public IPlugin {
    public:
        /** The id of the plugin */
        uint64_t m_id;

        /** The name of the plugin */
        std::string m_name;

        /** The path that the plugin was loaded from */
        fs::path m_filePath;

        /** The plugin's settings */
        PluginDescriptor m_descriptor;

        /** True if the plugin is marked as initialized */
        bool m_initialized{ false };

        /**
         * Plugin constructor
         */
        Plugin(uint64_t id, fs::path  filePath, PluginDescriptor  descriptor);

        /**
         * Destructor.
         */
        ~Plugin() override = default;

        /* IPlugin interface */
        uint64_t GetId() const override {
            return m_id;
        }

        const std::string& GetName() const override {
            return m_name;
        }

        const std::string& GetFriendlyName() const override {
            return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
        }

        const fs::path& GetDescriptorFilePath() const override {
            return m_filePath;
        }

        // TODO: Implement
        fs::path GetBaseDir() const override {
            return "";
        }
        fs::path GetContentDir() const override {
            return "";
        }
        fs::path GetMountedAssetPath() const override {
            return "";
        }

        const PluginDescriptor& GetDescriptor() const override {
            return m_descriptor;
        }

        bool IsInitialized() const { return m_initialized; }
        void SetInitialized(bool initialized) { m_initialized = initialized; }
    };
}
