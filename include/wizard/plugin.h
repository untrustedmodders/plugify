#pragma once

#include <cstdint>
#include <string>
#include <filesystem>
#include <wizard_export.h>

namespace wizard {
    class Plugin;
    struct PluginDescriptor;

    // Plugin provided to user, which implemented in core
    class WIZARD_API IPlugin {
    protected:
        explicit IPlugin(Plugin& impl);
        ~IPlugin();

    public:
        std::uint64_t GetId() const;
        const std::string& GetName() const;
        const std::string& GetFriendlyName() const;
        const std::filesystem::path& GetFilePath() const;
        std::filesystem::path GetBaseDir() const;
        std::filesystem::path GetContentDir() const;
        std::filesystem::path GetMountedAssetPath() const;
        const PluginDescriptor& GetDescriptor() const;

    private:
        Plugin& _impl;
    };
}
