#pragma once

namespace wizard {
    struct PluginDescriptor;

    // Plugin provided to user, which implemented in core
    class IPlugin {
    protected:
        ~IPlugin() = default;

    public:
        virtual uint64_t GetId() const = 0;
        virtual const std::string& GetName() const = 0;
        virtual const std::string& GetFriendlyName() const = 0;
        virtual const fs::path& GetDescriptorFilePath() const = 0;
        virtual fs::path GetBaseDir() const = 0;
        virtual fs::path GetContentDir() const = 0;
        virtual fs::path GetMountedAssetPath() const = 0;
        virtual const PluginDescriptor& GetDescriptor() const = 0;
    };
}
