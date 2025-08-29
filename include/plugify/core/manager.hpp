#pragma once

#include "plugify/core/dependency_resolver.hpp"
#include "plugify/core/file_system.hpp"
#include "plugify/core/manifest_parser.hpp"
#include "plugify/core/progress_reporter.hpp"
#include "plugify/core/types.hpp"

#include "service_locator.hpp"

namespace plugify {
    class Extension;
    class Context;
    class PLUGIFY_API Manager {
        struct Impl;
    public:
        explicit Manager(std::shared_ptr<Context> context);
        ~Manager();
        Manager(const Manager& other) = delete;
        Manager(Manager&& other) noexcept = delete;
        Manager& operator=(const Manager& other) = delete;
        Manager& operator=(Manager&& other) noexcept = delete;

        // Lifecycle
        void Initialize();
        void Update(Duration deltaTime);
        void Terminate();

        // Extension operations
        //Result<std::shared_ptr<Extension>> LoadExtension(const std::filesystem::path& path);
        //Result<void> UnloadExtension(std::string_view name);
        //Result<void> ReloadExtension(std::string_view name);
        //Result<void> EnableExtension(std::string_view name);
        //Result<void> DisableExtension(std::string_view name);

        // Query operations
        [[nodiscard]] bool IsExtensionLoaded(std::string_view name, std::optional<Constraint> version = {}) const noexcept;
        [[nodiscard]] std::shared_ptr<Extension> FindExtension(std::string_view name) const noexcept;
        [[nodiscard]] std::vector<std::shared_ptr<Extension>> GetExtensions() const;
        [[nodiscard]] std::vector<std::shared_ptr<Extension>> GetExtensionsByState(ExtensionState state) const;
        [[nodiscard]] std::vector<std::shared_ptr<Extension>> GetExtensionsByType(ExtensionType type) const;

        [[nodiscard]] bool operator==(const Manager& other) const noexcept;
        [[nodiscard]] auto operator<=>(const Manager& other) const noexcept;

    private:
        std::unique_ptr<Impl> _impl;
    };
}