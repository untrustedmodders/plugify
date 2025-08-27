#pragma once

#include "plugify/core/assembly.hpp"
#include "plugify/core/logger.hpp"
#include "plugify/core/service_locator.hpp"
#include "plugify/core/standart_file_system.hpp"

#include "event_bus.hpp"
#include "plugify_export.h"

namespace plugify {
	class Plugify;
    // Provider acts as a facade to simplify access to common services
    class PLUGIFY_API Provider {
        struct Impl;
    public:
        explicit Provider(std::shared_ptr<Context> context, std::shared_ptr<Manager> manager);
        ~Provider();

        // Logging helpers
        /*

        // Path helpers
        [[nodiscard]] const std::filesystem::path& GetBaseDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetPluginsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetConfigsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetDataDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetLogsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetCacheDir() const noexcept;*/

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


        void Log(std::string_view msg, Severity severity = Severity::Info) const;
        void LogVerbose(std::string_view msg) const { Log(msg, Severity::Verbose); }
        void LogDebug(std::string_view msg) const { Log(msg, Severity::Debug); }
        void LogInfo(std::string_view msg) const { Log(msg, Severity::Info); }
        void LogWarning(std::string_view msg) const { Log(msg, Severity::Warning); }
        void LogError(std::string_view msg) const { Log(msg, Severity::Error); }
        void LogFatal(std::string_view msg) const { Log(msg, Severity::Fatal); }
        void Log(Severity severity, std::string_view msg, const std::source_location& loc = std::source_location::current()) const;
        template<typename... Args>
        void LogFormat(Severity severity, std::string_view fmt, Args&&... args) const {
            Log(severity, std::format(fmt, std::forward<Args>(args)...));
        }

        // Path access from context
        [[nodiscard]] const std::filesystem::path& GetBaseDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetPluginsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetConfigsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetDataDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetLogsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetCacheDir() const noexcept;

        // Sandboxed file operations
        [[nodiscard]] Result<std::vector<uint8_t>> ReadBinaryFile(const std::filesystem::path& path) const;
        [[nodiscard]] Result<std::string> ReadTextFile(const std::filesystem::path& path) const;
        [[nodiscard]] Result<void> WriteBinaryFile(const std::filesystem::path& path,
                                            std::span<const uint8_t> data) const;
        [[nodiscard]] Result<void> WriteTextFile(const std::filesystem::path& path,
                                                std::string_view content) const;
        [[nodiscard]] Result<bool> FileExists(const std::filesystem::path& path) const;

        // Configuration access (read-only for security)
        template<typename T>
        [[nodiscard]] Result<T> GetConfig(std::string_view key) const {
            if (auto cfg =  GetContext()->GetConfigProvider()) {
                return cfg->GetAs<T>(key);
            }
            return plg::unexpected("Config provider not available");
        }

        // Plugin/Module queries
        [[nodiscard]] bool IsPluginLoaded(std::string_view name,
                                         std::optional<std::string> version = {}) const;
        [[nodiscard]] std::shared_ptr<Plugin> FindPlugin(std::string_view name) const;
        [[nodiscard]] std::vector<std::shared_ptr<Plugin>> GetLoadedPlugins() const;
        [[nodiscard]] bool IsModuleLoaded(std::string_view name,
                                         std::optional<std::string> version = {}) const;
        [[nodiscard]] std::shared_ptr<Module> FindModule(std::string_view name) const;
        [[nodiscard]] std::vector<std::shared_ptr<Module>> GetLoadedModules() const;

        // Event system access
        template<typename T>
        IEventBus::SubscriptionId Subscribe(std::function<void(const T&)> handler) {
            if (auto bus = GetContext()->GetEventBus()) {
                return bus->Subscribe<T>(std::move(handler));
            }
            return 0;
        }

        template<typename T>
        void Publish(T&& event) {
            if (auto bus = GetContext()->GetEventBus()) {
                bus->Publish(std::forward<T>(event));
            }
        }

        void Unsubscribe(IEventBus::SubscriptionId id) {
            if (auto bus = GetContext()->GetEventBus()) {
                bus->Unsubscribe(id);
            }
        }

    private:
        // Access to context for advanced use cases
        [[nodiscard]] std::shared_ptr<Context> GetContext() const noexcept;

    private:
        std::unique_ptr<Impl> _impl;
    };
}
