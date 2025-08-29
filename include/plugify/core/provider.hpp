#pragma once

#include "plugify/core/context.hpp"

#include "plugify_export.h"

namespace plugify {
    class Context;
    class Manager;
    class Extension;
    // Provider acts as a facade to simplify access to common services
    class PLUGIFY_API Provider {
        struct Impl;
    public:
        explicit Provider(std::shared_ptr<Context> context, std::shared_ptr<Manager> manager);
        ~Provider();
        Provider(const Provider& other) = delete;
        Provider(Provider&& other) noexcept = delete;
        Provider& operator=(const Provider& other) = delete;
        Provider& operator=(Provider&& other) noexcept = delete;

        // Logging helpers
        void Log(std::string_view msg, Severity severity = Severity::Info, const std::source_location& loc = std::source_location::current()) const;
        void LogVerbose(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Verbose, loc); }
        void LogDebug(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Debug, loc); }
        void LogInfo(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Info, loc); }
        void LogWarning(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Warning, loc); }
        void LogError(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Error, loc); }
        void LogFatal(std::string_view msg, const std::source_location& loc = std::source_location::current()) const { Log(msg, Severity::Fatal, loc); }
        /*template<typename... Args>
        void Log(Severity severity, std::format_string<Args...> fmt, Args&&... args) const {
            Log(std::format(fmt, std::forward<Args>(args)...), severity);
        }
        template<typename... Args>
        void LogVerbose(std::format_string<Args...> fmt, Args&&... args) const {
            Log(std::format(fmt, std::forward<Args>(args)...), Severity::Verbose);
        }
        template<typename... Args>
        void LogDebug(std::format_string<Args...> fmt, Args&&... args) const {
            Log(std::format(fmt, std::forward<Args>(args)...), Severity::Debug);
        }
        template<typename... Args>
        void LogInfo(std::format_string<Args...> fmt, Args&&... args) const {
            Log(std::format(fmt, std::forward<Args>(args)...), Severity::Info);
        }
        template<typename... Args>
        void LogWarning(std::format_string<Args...> fmt, Args&&... args) const {
            Log(std::format(fmt, std::forward<Args>(args)...), Severity::Warning);
        }
        template<typename... Args>
        void LogError(std::format_string<Args...> fmt, Args&&... args) const {
            Log(std::format(fmt, std::forward<Args>(args)...), Severity::Error);
        }
        template<typename... Args>
        void LogFatal(std::format_string<Args...> fmt, Args&&... args) const {
            Log(std::format(fmt, std::forward<Args>(args)...), Severity::Fatal);
        }*/

        bool IsPreferOwnSymbols() const noexcept;

        // Path helpers
        [[nodiscard]] const std::filesystem::path& GetBaseDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetExtensionsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetConfigsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetDataDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetLogsDir() const noexcept;
        [[nodiscard]] const std::filesystem::path& GetCacheDir() const noexcept;

        // Manager
        [[nodiscard]] bool IsExtensionLoaded(std::string_view name, std::optional<Constraint> version = {}) const noexcept;
        [[nodiscard]] std::shared_ptr<Extension> FindExtension(std::string_view name) const;
        [[nodiscard]] std::vector<std::shared_ptr<Extension>> GetExtensions() const;

        // Service access helpers
        template<typename Service>
        [[nodiscard]] std::shared_ptr<Service> GetService() const {
            return GetContext()->GetService<Service>();
        }

        [[nodiscard]] bool operator==(const Provider& other) const noexcept;
        [[nodiscard]] auto operator<=>(const Provider& other) const noexcept;

    private:
        // Access to context for advanced use cases
        [[nodiscard]] std::shared_ptr<Context> GetContext() const noexcept;

    private:
        std::unique_ptr<Impl> _impl;
    };
}
