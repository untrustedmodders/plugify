#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>
#include <memory>
#include <type_traits>

namespace plugify {
    /**
     * @enum ProtFlag
     * @brief Enum class representing various flags for dynamic library loading.
     */
    enum class LoadFlag : uint32_t {
        None = 0,
        LazyBinding = 1 << 0,   // Delay symbol resolution
        GlobalSymbols = 1 << 1, // Make symbols globally available
        NoUnload = 1 << 2,      // Prevent unloading
        DeepBind = 1 << 3,      // Prefer local symbols (Linux)
        DataOnly = 1 << 4,      // Load as data file (Windows)
        SecureSearch  = 1 << 5, // Use secure search paths (Windows)

        Default = LazyBinding
    };

    /**
     * @brief Bitwise OR operator for LoadFlag enum class.
     * @param lhs Left-hand side LoadFlag.
     * @param rhs Right-hand side LoadFlag.
     * @return Result of the bitwise OR operation.
     */
    inline LoadFlag operator|(LoadFlag lhs, LoadFlag rhs) noexcept {
        using underlying = std::underlying_type_t<LoadFlag>;
        return static_cast<LoadFlag>(
                static_cast<underlying>(lhs) | static_cast<underlying>(rhs)
        );
    }

    /**
     * @brief Bitwise AND operator for LoadFlag enum class.
     * @param lhs Left-hand side LoadFlag.
     * @param rhs Right-hand side LoadFlag.
     * @return Result of the bitwise AND operation.
     */
    inline bool operator&(LoadFlag lhs, LoadFlag rhs) noexcept {
        using underlying = std::underlying_type_t<LoadFlag>;
        return static_cast<underlying>(lhs) & static_cast<underlying>(rhs);
    }

    /**
     * @brief Bitwise OR assignment operator for LoadFlag enum class.
     * @param lhs Left-hand side LoadFlag.
     * @param rhs Right-hand side LoadFlag.
     * @return Reference to the left-hand side LoadFlag.
     */
    inline LoadFlag& operator|=(LoadFlag& lhs, LoadFlag rhs) noexcept {
        lhs = lhs | rhs;
        return lhs;
    }
}