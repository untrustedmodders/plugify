#include "file_system.h"

using namespace wizard;

void FileSystem::ReadBytes(const fs::path& filepath, const FileHandler& handler) {
    std::basic_ifstream<uint8_t, std::char_traits<uint8_t>> is{filepath, std::ios::binary};

    if (!is.is_open()) {
        WIZARD_LOG("File: '" + filepath.string() + "' could not be opened", ErrorLevel::ERROR);
        return;
    }

    // Stop eating new lines in binary mode!!!
    is.unsetf(std::ios::skipws);

    std::vector<uint8_t> buffer{ std::istreambuf_iterator<uint8_t>{is}, std::istreambuf_iterator<uint8_t>{} };
    handler({ buffer.data(), buffer.size() });
}

std::string FileSystem::ReadText(const fs::path& filepath) {
    std::ifstream is{filepath, std::ios::binary};

    if (!is.is_open()) {
        WIZARD_LOG("File: '" + filepath.string() + "' could not be opened", ErrorLevel::ERROR);
        return {};
    }

    // Stop eating new lines in binary mode!!!
    is.unsetf(std::ios::skipws);

    return { std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{} };
}

bool FileSystem::IsExists(const fs::path& filepath) {
    return fs::exists(filepath);
}

bool FileSystem::IsDirectory(const fs::path& filepath) {
    return fs::is_directory(filepath);
}

std::vector<fs::path> FileSystem::GetFiles(const fs::path& root, bool recursive, std::string_view ext) {
    std::vector<fs::path> paths;

    if (fs::exists(root) && fs::is_directory(root)) {
        auto iterate = [&paths, ext](auto iterator) {
            for (auto const& entry : iterator) {
                const auto& path = entry.path();
                if (fs::is_regular_file(entry) && (ext.empty() || path.extension().string() == ext)) {
                    paths.push_back(path);
                }
            }
        };

        if (recursive) {
            iterate(fs::recursive_directory_iterator(root));
        } else {
            iterate(fs::directory_iterator(root));
        }
    }

    return paths;
}