#include "plugify/core/file_system.hpp"

using namespace plugify;

Result<std::string> StandardFileSystem::ReadTextFile(const std::filesystem::path& path) {
    std::error_code ec;
    
    if (!std::filesystem::exists(path, ec)) {
        return plg::unexpected(std::format("file not found: {}", ec.message()));
    }

    if (!std::filesystem::is_regular_file(path, ec)) {
        return plg::unexpected(std::format("not a regular file: {}", ec.message()));
    }

    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        return plg::unexpected(std::format("cannot open file: {}", SystemError()));
    }

    // Read entire file
    std::stringstream buffer;
    buffer << file.rdbuf();

    if (!file && !file.eof()) {
        return plg::unexpected(std::format("error reading file: {}", SystemError()));
    }

    return buffer.str();
}

Result<std::vector<uint8_t>> StandardFileSystem::ReadBinaryFile(const std::filesystem::path& path) {
    std::error_code ec;

    if (!std::filesystem::exists(path, ec)) {
        return plg::unexpected(std::format("file not found: {}", ec.message()));
    }

    auto fileSize = std::filesystem::file_size(path, ec);
    if (ec) {
        return plg::unexpected(std::format("cannot get file size: {}", ec.message()));
    }

    std::vector<uint8_t> data(fileSize);
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return plg::unexpected(std::format("cannot open file: {}", SystemError()));
    }

    file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(fileSize));
    if (!file && !file.eof()) {
        return plg::unexpected(std::format("error reading file: {}", SystemError()));
    }

    return data;
}

Result<void> StandardFileSystem::WriteTextFile(const std::filesystem::path& path, std::string_view content) {
    std::error_code ec;

    // Create parent directories if needed
    auto parent = path.parent_path();
    if (!parent.empty() && !std::filesystem::exists(parent, ec)) {
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            return plg::unexpected(std::format("cannot create parent directory: {}", ec.message()));
        }
    }

    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file) {
        return plg::unexpected(std::format("cannot open file for writing: {}", SystemError()));
    }

    file << content;
    if (!file) {
        return plg::unexpected(std::format("error writing file: {}", SystemError()));
    }

    return {};
}

Result<void> StandardFileSystem::WriteBinaryFile(const std::filesystem::path& path, std::span<const uint8_t> data) {
    std::error_code ec;

    // Create parent directories if needed
    auto parent = path.parent_path();
    if (!parent.empty() && !std::filesystem::exists(parent, ec)) {
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            return plg::unexpected(std::format("cannot create parent directory: {}", ec.message()));
        }
    }

    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        return plg::unexpected(std::format("cannot open file for writing: {}", SystemError()));
    }

    file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!file) {
        return plg::unexpected(std::format("error writing file: {}", SystemError()));
    }

    return {};
}

bool StandardFileSystem::IsExists(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

bool StandardFileSystem::IsDirectory(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::is_directory(path, ec);
}

bool StandardFileSystem::IsRegularFile(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec);
}

Result<FileInfo> StandardFileSystem::GetFileInfo(const std::filesystem::path& path) {
    std::error_code ec;

    if (!std::filesystem::exists(path, ec)) {
        return plg::unexpected(std::format("path not found: {}", ec.message()));
    }

    FileInfo info {
        .path = path,
        .lastModified = std::filesystem::last_write_time(path, ec),
        .size = std::filesystem::is_regular_file(path, ec) ? std::filesystem::file_size(path, ec) : 0,
        .isDirectory = std::filesystem::is_directory(path, ec),
        .isRegularFile = std::filesystem::is_regular_file(path, ec),
        .isSymlink = std::filesystem::is_symlink(path, ec),
    };

    if (ec) {
        return plg::unexpected(std::format("error getting file info: {}", ec.message()));
    }

    return info;
}

Result<std::vector<FileInfo>> StandardFileSystem::ListDirectory(const std::filesystem::path& directory) {
    std::error_code ec;

    if (!std::filesystem::is_directory(directory, ec)) {
        return plg::unexpected(std::format("not a directory: {}", ec.message()));
    }

    std::vector<FileInfo> files;

    for (const auto& entry : std::filesystem::directory_iterator(directory, ec)) {
        if (ec) {
            return plg::unexpected(std::format("error iterating directory: {}", ec.message()));
        }

		if (auto infoResult = GetFileInfo(entry.path())) {
            files.push_back(std::move(*infoResult));
        }
    }

    return files;
}

Result<std::vector<FileInfo>> StandardFileSystem::IterateDirectory(
    const std::filesystem::path& directory,
    const DirectoryIterationOptions& options) {

    std::error_code ec;

    if (!std::filesystem::is_directory(directory, ec)) {
        return plg::unexpected(std::format("not a directory: {}", ec.message()));
    }

    std::vector<FileInfo> files;

    if (options.recursive) {
        IterateRecursive(directory, files, options, 0, ec);
    } else {
        for (const auto& entry : std::filesystem::directory_iterator(directory, ec)) {
            if (ec) break;

            auto infoResult = GetFileInfo(entry.path());
            if (infoResult && ShouldInclude(*infoResult, options)) {
                files.push_back(std::move(*infoResult));
            }
        }
    }

    if (ec) {
        return plg::unexpected(std::format("error iterating directory: {}", ec.message()));
    }

    return files;
}

Result<std::vector<std::filesystem::path>> StandardFileSystem::FindFiles(
    const std::filesystem::path& directory,
    std::initializer_list<std::string_view> patterns,
    bool recursive) {

    std::error_code ec;

    if (!std::filesystem::is_directory(directory, ec)) {
        return plg::unexpected(std::format("not a directory: {}", ec.message()));
    }

    std::vector<std::filesystem::path> matches;

    if (!recursive) {
        for (const auto& entry : std::filesystem::directory_iterator(directory, ec)) {
            if (ec) break;
            if (MatchesPatterns(entry.path().filename().string(), patterns)) {
                matches.push_back(entry.path());
            }
        }
    } else {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory, ec)) {
            if (ec) break;
            if (MatchesPatterns(entry.path().filename().string(), patterns)) {
                matches.push_back(entry.path());
            }
        }
    }

    if (ec) {
        return plg::unexpected(std::format("error finding files: {}", ec.message()));
    }

    return matches;
}

Result<void> StandardFileSystem::CreateDirectories(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    if (ec) {
        return plg::unexpected(std::format("cannot create directories: {}", ec.message()));
    }
    return {};
}

Result<void> StandardFileSystem::Remove(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
    if (ec) {
        return plg::unexpected(std::format("cannot remove path: {}", ec.message()));
    }
    return {};
}

Result<void> StandardFileSystem::RemoveAll(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
    if (ec) {
        return plg::unexpected(std::format("cannot remove directory: {}", ec.message()));
    }
    return {};
}

Result<void> StandardFileSystem::Copy(const std::filesystem::path& from, const std::filesystem::path& to) {
    std::error_code ec;
    std::filesystem::copy(from, to, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        return plg::unexpected(std::format("cannot copy: {}", ec.message()));
    }
    return {};
}

Result<void> StandardFileSystem::Move(const std::filesystem::path& from, const std::filesystem::path& to) {
    std::error_code ec;
    std::filesystem::rename(from, to, ec);
    if (ec) {
        return plg::unexpected(std::format("cannot move: {}", ec.message()));
    }
    return {};
}

std::string StandardFileSystem::SystemError() {
	std::string result(1024, '\0');
	// TODO: Validate that it cross-platform
#if PLUGIFY_PLATFORM_WINDOWS
	strerror_s(result.data(), result.size(), errno);
#elif PLUGIFY_PLATFORM_APPLE
	strerror_r(errno, result.data(), result.size());
#else
	auto b = result.data();
	auto r = strerror_r(errno, b, result.size());
	if (r != b) return r;
#endif
	result.resize(strlen(result.data()));
	return result;
}

void StandardFileSystem::IterateRecursive(
    const std::filesystem::path& directory,
    std::vector<FileInfo>& files,
    const DirectoryIterationOptions& options,
    size_t currentDepth,
    std::error_code& ec) {

    if (options.maxDepth && currentDepth >= *options.maxDepth) {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory, ec)) {
        if (ec) return;
        
        auto infoResult = GetFileInfo(entry.path());
        if (!infoResult) continue;
        
        if (ShouldInclude(*infoResult, options)) {
            files.push_back(std::move(*infoResult));
        }
        
        if (infoResult->isDirectory && !infoResult->isSymlink) {
            IterateRecursive(entry.path(), files, options, currentDepth + 1, ec);
        }
    }
}

bool StandardFileSystem::ShouldInclude(const FileInfo& info, const DirectoryIterationOptions& options) {
    // Check extension filter
    if (!options.extensions.empty() && info.isRegularFile) {
        bool hasMatchingExt = false;
        for (const auto& ext : options.extensions) {
            if (info.path.extension().string() == ext) {
                hasMatchingExt = true;
                break;
            }
        }
        if (!hasMatchingExt) return false;
    }
    
    // Check custom filter
    if (options.filter && !options.filter(info)) {
        return false;
    }
    
    return true;
}

bool StandardFileSystem::MatchesPatterns(std::string_view filename, std::initializer_list<std::string_view> patterns) {
    if (patterns.size() == 0) return true;
    
    for (const auto& pattern : patterns) {
        if (SimpleMatch(filename, pattern)) {
            return true;
        }
    }
    
    return false;
}

bool StandardFileSystem::SimpleMatch(std::string_view text, std::string_view pattern) {
    // Simple wildcard matching (supports * and ?)
    size_t textPos = 0;
    size_t patternPos = 0;
    size_t starPos = std::string::npos;
    size_t matchPos = 0;
    
    while (textPos < text.length()) {
        if (patternPos < pattern.length() && 
            (pattern[patternPos] == '?' || pattern[patternPos] == text[textPos])) {
            textPos++;
            patternPos++;
        } else if (patternPos < pattern.length() && pattern[patternPos] == '*') {
            starPos = patternPos++;
            matchPos = textPos;
        } else if (starPos != std::string::npos) {
            patternPos = starPos + 1;
            textPos = ++matchPos;
        } else {
            return false;
        }
    }
    
    while (patternPos < pattern.length() && pattern[patternPos] == '*') {
        patternPos++;
    }
    
    return patternPos == pattern.length();
}
