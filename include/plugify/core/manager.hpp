#pragma once

#include <deque>
#include <ranges>

#include "plugify/core/dependency_resolver.hpp"
#include "plugify/core/manifest_parser.hpp"
#include "plugify/core/progress_reporter.hpp"
#include "plugify/core/standart_file_system.hpp"
#include "plugify/core/types.hpp"

#include "service_locator.hpp"

namespace plugify {
    class PLUGIFY_API PluginManager {
        struct Impl;
    public:
        explicit PluginManager(ServiceLocator services);
        ~PluginManager();

        // Plugin operations
        Result<std::shared_ptr<Plugin>> LoadPlugin(const std::filesystem::path& path);
        Result<void> UnloadPlugin(std::string_view name);
        Result<void> ReloadPlugin(std::string_view name);
        Result<void> EnablePlugin(std::string_view name);
        Result<void> DisablePlugin(std::string_view name);

        // Module operations
        Result<std::shared_ptr<Module>> LoadModule(const std::filesystem::path& path);
        Result<void> UnloadModule(std::string_view name);
        Result<void> ReloadModule(std::string_view name);
        Result<void> EnableModule(std::string_view name);
        Result<void> DisableModule(std::string_view name);

        // Bulk operations
        Result<void> LoadAllPlugins();
        Result<void> UnloadAllPlugins();

        // Query operations
        [[nodiscard]] bool IsPluginLoaded(std::string_view name,
                                          std::optional<Constraint> version = {}) const noexcept;
        [[nodiscard]] std::shared_ptr<Plugin> FindPlugin(std::string_view name) const noexcept;
        [[nodiscard]] std::vector<std::shared_ptr<Plugin>> GetPlugins() const;
        [[nodiscard]] std::vector<std::shared_ptr<Plugin>> GetPluginsByState(PluginState state) const;

        // Module operations
        Result<std::shared_ptr<Module>> LoadModule(const std::filesystem::path& path);
        Result<void> UnloadModule(std::string_view name);
        [[nodiscard]] bool IsModuleLoaded(std::string_view name,
                                          std::optional<Constraint> version = {}) const noexcept;
        [[nodiscard]] std::shared_ptr<Module> FindModule(std::string_view name) const noexcept;
        [[nodiscard]] std::vector<std::shared_ptr<Module>> GetModules() const;
        [[nodiscard]] std::vector<std::shared_ptr<Module>> GetPluginsByState(ModuleState state) const;

        // Lifecycle
        void Update();

    private:
        std::unique_ptr<Impl> _impl;
    };
}