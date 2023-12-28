#pragma once

#include <wizard/plugin.h>
#include <wizard/plugin_descriptor.h>

namespace wizard {
    enum class PluginState : uint8_t {
        NotLoaded,
        Error,
        Loaded,
        Running,
        Terminating,
        Unloaded
    };

    class Module;
    class Plugin final : public IPlugin {
    public:
        Plugin(uint64_t id, std::string name, fs::path filePath, PluginDescriptor descriptor);
        ~Plugin() = default;

        /* IPlugin interface */
        uint64_t GetId() const {
            return _id;
        }

        const std::string& GetName() const {
            return _name;
        }

        const std::string& GetFriendlyName() const {
            return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
        }

        const fs::path& GetFilePath() const {
            return _filePath;
        }

        fs::path GetBaseDir() const {
            return "";
        }
        fs::path GetContentDir() const {
            return "";
        }
        fs::path GetMountedAssetPath() const {
            return "";
        }

        const PluginDescriptor& GetDescriptor() const {
            return _descriptor;
        }

        std::shared_ptr<Module> GetModule() const {
            return _module;
        }

        void SetModule(std::shared_ptr<Module> module) {
            _module = std::move(module);
        }

        PluginState GetState() const {
            return _state;
        }

        void SetError(std::string error);

        void SetLoaded() {
            _state = PluginState::Loaded;
        }

        void SetRunning() {
            _state = PluginState::Running;
        }

        void SetTerminating() {
            _state = PluginState::Terminating;
        }

        void SetUnloaded() {
            _state = PluginState::Unloaded;
        }

        static inline const char* const kFileExtension = ".wplugin";

    private:
        uint64_t _id{ std::numeric_limits<uint64_t>::max() };
        std::string _name;
        fs::path _filePath;
        std::string _error;
        std::shared_ptr<Module> _module;
        PluginDescriptor _descriptor;
        PluginState _state{ PluginState::NotLoaded };
    };
}
