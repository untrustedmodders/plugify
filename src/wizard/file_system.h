#pragma once

#include <fstream>

namespace wizard {
    class FileSystem {
    public:
        template<typename T>
        static inline bool ReadBytes(const fs::path& file, const std::function<void(std::span<T>)>& callback) {
            std::ifstream is{file, std::ios::binary};
            if (!is) return false;
            std::vector<T> buffer{ std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>() };
            callback({ buffer.data(), buffer.size() });
            return true;
        }
    };
}