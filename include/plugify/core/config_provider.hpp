#pragma once

#include <any>
#include <filesystem>
#include <string>

#include "plugify/core/types.hpp"

namespace plugify {
    // Configuration interface
    /*class IConfigProvider {
    public:
        virtual ~IConfigProvider() = default;
        virtual Result<std::any> GetValue(std::string_view key) = 0;
        virtual Result<void> SetValue(std::string_view key, std::any value) = 0;
        virtual Result<void> LoadFromFile(const std::filesystem::path& path) = 0;
        virtual Result<void> SaveToFile(const std::filesystem::path& path) = 0;
        [[nodiscard]] virtual bool IsDirty() const = 0;

        // Type-safe getters using concepts
        template<typename T>
        Result<T> GetAs(std::string_view key) {
            auto result = GetValue(key);
            if (!result) return plg::unexpected(result.error());

            try {
                return std::any_cast<T>(*result);
            } catch (const std::bad_any_cast&) {
                return plg::unexpected("Type mismatch for config key");
            }
        }
    };*/
}