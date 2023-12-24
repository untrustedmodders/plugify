#pragma once

#include <cstdint>
#include <string>
#include <filesystem>

namespace wizard {
    struct PluginDescriptor;

    // Plugin provided to user, which implemented in core
    class IPlugin {
    protected:
        ~IPlugin() = default;

    public:
        virtual std::uint64_t GetId() const = 0;
        virtual const std::string& GetName() const = 0;
        virtual const std::string& GetFriendlyName() const = 0;
        virtual const std::filesystem::path& GetDescriptorFilePath() const = 0;
        virtual std::filesystem::path GetBaseDir() const = 0;
        virtual std::filesystem::path GetContentDir() const = 0;
        virtual std::filesystem::path GetMountedAssetPath() const = 0;
        virtual const PluginDescriptor& GetDescriptor() const = 0;
    };
}
