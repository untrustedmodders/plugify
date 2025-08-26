#pragma once

#include "plugify/core/assembly.hpp"
#include "plugify/core/logger.hpp"
#include "plugify/core/service_locator.hpp"
#include "plugify/core/standart_file_system.hpp"

#include "plugify_export.h"

namespace plugify {
	class Plugify;
    // Provider acts as a facade to simplify access to common services
    class PLUGIFY_API Provider {
        struct Impl;
    public:
        explicit Provider(std::weak_ptr<ServiceLocator> services);
        ~Provider();

        // Logging helpers
        void Log(std::string_view msg, Severity severity = Severity::Info) const;
        void LogVerbose(std::string_view msg) const { Log(msg, Severity::Verbose); }
        void LogDebug(std::string_view msg) const { Log(msg, Severity::Debug); }
        void LogInfo(std::string_view msg) const { Log(msg, Severity::Info); }
        void LogWarning(std::string_view msg) const { Log(msg, Severity::Warning); }
        void LogError(std::string_view msg) const { Log(msg, Severity::Error); }
        void LogFatal(std::string_view msg) const { Log(msg, Severity::Fatal); }

        // Path helpers
        [[nodiscard]] const std::filesystem::path& GetBaseDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetPluginsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetConfigsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetDataDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetLogsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetCacheDir() const noexcept;

        // File operations (sandboxed for security)
        /*[[nodiscard]] Result<std::vector<uint8_t>> ReadFile(const std::filesystem::path& path) const = 0;
        [[nodiscard]] Result<void> WriteFile(const std::filesystem::path& path,
                                      const std::vector<uint8_t>& data) const = 0;
        [[nodiscard]] Result<bool> FileExists(const std::filesystem::path& path) const = 0;

        // Configuration access (read-only for modules)
        [[nodiscard]] Result<std::any> GetConfig(std::string_view key) const = 0;
        [[nodiscard]] Result<std::string> GetConfigString(std::string_view key) const = 0;
        [[nodiscard]] Result<int64_t> GetConfigInt(std::string_view key) const = 0;
        [[nodiscard]]  [[nodiscard]] Result<bool> GetConfigBool(std::string_view key) const = 0;

        // Manager
        [[nodiscard]] bool IsPluginLoaded(std::string_view name,
                                          std::optional<Constraint> version = {}) const noexcept;
        [[nodiscard]] std::shared_ptr<Plugin> FindPlugin(std::string_view name) const;
        [[nodiscard]] std::vector<std::shared_ptr<Plugin>> GetLoadedPlugins() const;

        [[nodiscard]] bool IsModuleLoaded(std::string_view name,
                                          std::optional<Constraint> version = {}) const noexcept;
        [[nodiscard]] std::shared_ptr<Module> FindPlugin(std::string_view name) const;
        [[nodiscard]] std::vector<std::shared_ptr<Module>> GetLoadedModules() const;*/

        // Service access helpers
       /*template<typename Service>
        [[nodiscard]] std::shared_ptr<Service> GetService() const {
            if (auto services = _services.lock()) {
                return services->Get<Service>();
            }
            return nullptr;
        }

        // Event helpers
        template<typename EventType>
        void PublishEvent(const EventType& event) const {
            if (auto eventBus = GetService<IEventBus>()) {
                eventBus->Publish(event);
            }
        }*/
    private:
        std::unique_ptr<Impl> _impl;
    };
}
