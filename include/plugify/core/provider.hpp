#pragma once

#include "plugify/core/types.hpp"
#include "plugify/core/logger.hpp"
#include "plugify/core/service_locator.hpp"

#include "plugify_export.h"

namespace plugify {
    struct Config;
    class Manager;
    class Extension;
    // Provider acts as a facade to simplify access to common services
    class PLUGIFY_API Provider {
        struct Impl;
    public:
        explicit Provider(const ServiceLocator& services, const Config& config, const Manager& manager);
        ~Provider();
        Provider(const Provider& other);
        Provider(Provider&& other) noexcept;
        Provider& operator=(const Provider& other);
        Provider& operator=(Provider&& other) noexcept;

        // Logging helpers
        void Log(std::string_view msg, Severity severity = Severity::Info, const std::source_location& loc = std::source_location::current()) const;
        void LogVerbose(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Verbose, loc); }
        void LogDebug(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Debug, loc); }
        void LogInfo(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Info, loc); }
        void LogWarning(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Warning, loc); }
        void LogError(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Error, loc); }
        void LogFatal(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Fatal, loc); }

        bool IsPreferOwnSymbols() const noexcept;

        // Path helpers
        [[nodiscard]] const std::filesystem::path& GetBaseDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetExtensionsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetConfigsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetDataDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetLogsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetCacheDir() const noexcept;

        // Manager
        [[nodiscard]] bool IsExtensionLoaded(std::string_view name, std::optional<Constraint> constraint = {}) const noexcept;
        [[nodiscard]] const Extension* FindExtension(std::string_view name) const;
        [[nodiscard]] const Extension* FindExtension(UniqueId id) const;
        [[nodiscard]] std::vector<const Extension*> GetExtensions() const;

        // Service access helpers
        template<typename Service>
        [[nodiscard]] std::shared_ptr<Service> GetService() const {
            return GetServices().Get<Service>();
        }

        [[nodiscard]] bool operator==(const Provider& other) const noexcept;
        [[nodiscard]] auto operator<=>(const Provider& other) const noexcept;

    private:
        // Access to context for advanced use cases
        [[nodiscard]] const ServiceLocator& GetServices() const noexcept;

    private:
        std::unique_ptr<Impl> _impl;
    };
}
