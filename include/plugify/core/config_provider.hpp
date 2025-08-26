#pragma once

#include <any>
#include <string>
#include <filesystem>

namespace plugify {
    // Configuration interface
    class IConfigProvider {
    public:
        virtual ~IConfigProvider() = default;
        virtual Result<std::any> GetValue(std::string_view key) = 0;
        virtual Result<void> SetValue(std::string_view key, std::any value) = 0;
        virtual Result<void> LoadFromFile(const std::filesystem::path& path) = 0;
        virtual Result<void> SaveToFile(const std::filesystem::path& path) = 0;
        [[nodiscard]] virtual bool IsDirty() const = 0;
    };
}