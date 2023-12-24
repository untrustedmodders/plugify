#pragma once

namespace wizard {
    class Paths {
    public:
        Paths() = delete;

        static const fs::path& ModulesDir();
        static const fs::path& PluginsDir();
    };

    // std::filesystem::hash_value not in the C++20 standard by default
    struct PathHash {
        auto operator()(const fs::path& path) const noexcept {
            return fs::hash_value(path);
        }
    };
}