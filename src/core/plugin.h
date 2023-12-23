#pragma once

#include <wizard/plugin.h>
#include "plugin_descriptor.h"

namespace wizard {
    class Plugin final : public IPlugin {
    public:
        Plugin(uint64_t id, fs::path filePath, PluginDescriptor descriptor);
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

    private:
        uint64_t m_id;
        std::string m_name;
        fs::path m_filePath;
        PluginDescriptor m_descriptor;
        bool m_initialized{ false };
    };
}
