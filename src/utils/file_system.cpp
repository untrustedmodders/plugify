#include "file_system.h"

using namespace wizard;

void FileSystem::ReadBytes(const fs::path& filepath, const FileHandler& handler) {
    std::ifstream is{filepath, std::ios::binary};

    if (!is.is_open()) {
        WIZARD_LOG("File: '" + filepath.string() + "' could not be opened", ErrorLevel::SEV);
        return;
    }

    // Stop eating new lines in binary mode!!!
    is.unsetf(std::ios::skipws);

    is.seekg(0, std::ios::end);
    std::streampos size = is.tellg();
    is.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer;
    buffer.reserve(static_cast<size_t>(size));
    buffer.insert(buffer.begin(), std::istream_iterator<uint8_t>(is), std::istream_iterator<uint8_t>());

    handler({ buffer.data(), buffer.size() });
}

std::string FileSystem::ReadText(const fs::path& filepath) {
    std::ifstream is{filepath, std::ios::binary};

    if (!is.is_open()) {
        WIZARD_LOG("File: '" + filepath.string() + "' could not be opened", ErrorLevel::SEV);
        return {};
    }

    // Stop eating new lines in binary mode!!!
    is.unsetf(std::ios::skipws);

    return { std::istream_iterator<int8_t>(is), std::istream_iterator<int8_t>() };
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
        if (!ext.empty()) {
            for (auto const& entry : fs::recursive_directory_iterator(root)) {
                const auto& path = entry.path();
                if (fs::is_regular_file(entry) && Paths::GetExtension(path) == ext)
                    paths.push_back(path);
            }
        } else {
            for (auto const& entry : fs::recursive_directory_iterator(root)) {
                if (fs::is_regular_file(entry))
                    paths.push_back(entry.path());
            }
        }
    }

    return paths;
}