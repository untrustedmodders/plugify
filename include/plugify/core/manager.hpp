#pragma once

#include "plugify/core/types.hpp"

namespace plugify {
    struct Config;
    class Extension;
    class ServiceLocator;
    class PLUGIFY_API Manager {
        struct Impl;
    public:
        explicit Manager(const ServiceLocator& services, const Config& config);
        ~Manager();
        Manager(const Manager& other) = delete;
        Manager(Manager&& other) noexcept = delete;
        Manager& operator=(const Manager& other) = delete;
        Manager& operator=(Manager&& other) noexcept = delete;

        // Lifecycle
        void Initialize() const;
        [[nodiscard]] bool IsInitialized() const;
        void Update(Duration deltaTime) const;
        void Terminate() const;

        // Extension operations
        //Result<ExtensionRef> LoadExtension(const std::filesystem::path& path);
        //Result<void> UnloadExtension(std::string_view name);
        //Result<void> ReloadExtension(std::string_view name);
        //Result<void> EnableExtension(std::string_view name);
        //Result<void> DisableExtension(std::string_view name);

        // Query operations
        [[nodiscard]] bool IsExtensionLoaded(std::string_view name, std::optional<Constraint> constraint = {}) const noexcept;
        [[nodiscard]] const Extension* FindExtension(std::string_view name) const noexcept;
        [[nodiscard]] std::span<const Extension> GetExtensions() const;
        [[nodiscard]] std::vector<const Extension*> GetExtensionsByState(ExtensionState state) const;
        [[nodiscard]] std::vector<const Extension*> GetExtensionsByType(ExtensionType type) const;

        [[nodiscard]] bool operator==(const Manager& other) const noexcept;
        [[nodiscard]] auto operator<=>(const Manager& other) const noexcept;

    private:
        std::unique_ptr<Impl> _impl;
    };
}