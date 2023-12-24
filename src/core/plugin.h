#pragma once

#include <wizard/plugin.h>
#include "plugin_descriptor.h"

namespace wizard {
    class Plugin final : public IPlugin {
    public:
        Plugin(uint64_t id, std::string name, fs::path filePath, PluginDescriptor descriptor);
        ~Plugin() = default;

        /* IPlugin interface */
        uint64_t GetId() const override {
            return _id;
        }

        const std::string& GetName() const override {
            return _name;
        }

        const std::string& GetFriendlyName() const override {
            return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
        }

        const fs::path& GetDescriptorFilePath() const override {
            return _filePath;
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
            return _descriptor;
        }

        bool IsInitialized() const { return _initialized; }
        void SetInitialized(bool initialized) { _initialized = initialized; }

    private:
        uint64_t _id{ std::numeric_limits<uint64_t>::max() };
        std::string _name;
        fs::path _filePath;
        PluginDescriptor _descriptor;
        bool _initialized{ false };
    };
}
