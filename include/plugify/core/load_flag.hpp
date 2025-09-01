#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>
#include <memory>

namespace plugify {
    using SearchPaths = std::vector<std::filesystem::path>;

    // Simplified, unified load flags across platforms
    enum class LoadFlag : uint32_t {
        None = 0,
        LazyBinding = 1 << 0,      // Delay symbol resolution
        GlobalSymbols = 1 << 1,     // Make symbols globally available
        NoUnload = 1 << 2,          // Prevent unloading
        DeepBind = 1 << 3,          // Prefer local symbols (Linux)
        DataOnly = 1 << 4,          // Load as data file (Windows)
        SystemOnly = 1 << 5,        // Search system dirs only

        Default = LazyBinding
    };

    // Bitwise operators for LoadFlag
    inline LoadFlag operator|(LoadFlag a, LoadFlag b) {
        return static_cast<LoadFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline bool operator&(LoadFlag a, LoadFlag b) {
        return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
    }
}