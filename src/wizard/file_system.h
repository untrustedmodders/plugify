#pragma once

#include <fstream>

namespace wizard {
    class FileSystem {
    public:
        template<typename T>
        static inline bool ReadBytes(const fs::path& file, const std::function<void(std::span<T>)>& callback) {
            std::ifstream istream{file, std::ios::binary};
            if (!istream)
                return false;
            std::vector<T> buffer{ std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>() };
            callback({ buffer.data(), buffer.size() });
            return true;
        }
    };
}