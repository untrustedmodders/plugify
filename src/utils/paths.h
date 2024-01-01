#pragma once

namespace fs = std::filesystem;

namespace wizard {
    // std::hash<std::filesystem::path> not in the C++20 standard by default
    struct PathHash {
        auto operator()(const fs::path& path) const noexcept {
            return fs::hash_value(path);
        }
    };
}