#include "virtual_file_system.h"

#include <physfs.h>

using namespace wizard;

void VirtualFileSystem::Initialize() {
    // TODO: This may be NULL on most platforms (such as ones without a standard main() function), but you should always try to pass something in here. Unix-like systems such as Linux need to pass argv[0] from main() in here.
    PHYSFS_init(NULL);
}

void VirtualFileSystem::Shutdown() {
    PHYSFS_deinit();
}

void VirtualFileSystem::Mount(const fs::path& path, std::string_view mount) {
    PHYSFS_mount(path.generic_string().c_str(), mount.data(), 1);
}

void VirtualFileSystem::Unmount(const fs::path& path) {
    PHYSFS_unmount(path.generic_string().c_str());
}

void VirtualFileSystem::ReadBytes(const fs::path& filepath, const FileHandler& handler) {
    if (!PHYSFS_isInit()) {
        WZ_LOG_FATAL("PHYSFS library was not initialized");
        return;
    }

    auto fsFile = PHYSFS_openRead(filepath.generic_string().c_str());

    if (!fsFile) {
        WZ_LOG_ERROR("Failed to open file: '{}' - {}", filepath.string(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return;
    }

    auto size = static_cast<size_t>(PHYSFS_fileLength(fsFile));
    std::vector<uint8_t> buffer(size);
    PHYSFS_readBytes(fsFile, buffer.data(), size);

    if (PHYSFS_close(fsFile) == 0) {
        WZ_LOG_ERROR("Failed to close file: '{}' - {}", filepath.string(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    handler(buffer);
}

std::string VirtualFileSystem::ReadText(const fs::path& filepath) {
    if (!PHYSFS_isInit()) {
        WZ_LOG_FATAL("PHYSFS library was not initialized");
        return {};
    }

    auto fsFile = PHYSFS_openRead(filepath.generic_string().c_str());

    if (!fsFile) {
        WZ_LOG_ERROR("Failed to open file: '{}' - {}", filepath.string(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return {};
    }

    auto size = static_cast<size_t>(PHYSFS_fileLength(fsFile));
    std::vector<uint8_t> buffer(size);
    PHYSFS_readBytes(fsFile, buffer.data(), size);

    if (PHYSFS_close(fsFile) == 0) {
        WZ_LOG_ERROR("Failed to close file: '{}' - {}", filepath.string(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    return { buffer.begin(), buffer.end() };
}

bool VirtualFileSystem::IsExists(const fs::path& filepath) {
    if (!PHYSFS_isInit()) {
        WZ_LOG_FATAL("PHYSFS library was not initialized");
        return false;
    }

    return PHYSFS_exists(filepath.generic_string().c_str()) != 0;
}

bool VirtualFileSystem::IsDirectory(const fs::path& filepath) {
    if (!PHYSFS_isInit()) {
        WZ_LOG_FATAL("PHYSFS library was not initialized");
        return false;
    }

    std::string directory{ filepath.generic_string() };

    PHYSFS_Stat stat;
    if (!PHYSFS_stat(directory.c_str(), &stat)) {
        return false;
    } else {
        if (stat.filetype == PHYSFS_FILETYPE_SYMLINK) {
            // PHYSFS_stat() doesn't follow symlinks, so we do it manually
            const char* realdir = PHYSFS_getRealDir(directory.c_str());
            if (realdir == nullptr) {
                return false;
            } else {
                return IsDirectory(fs::path{realdir} / filepath);
            }
        } else {
            return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
        }
    }
}

bool VirtualFileSystem::GetFilesFromDirectoryRecursive(const fs::path& directory, std::unordered_map<fs::path, fs::path, PathHash>& results, std::string_view ext) {
    if (!PHYSFS_isInit()) {
        WZ_LOG_FATAL("PHYSFS library was not initialized");
        return false;
    }

    auto rc = PHYSFS_enumerateFiles(directory.generic_string().c_str());

    for (auto i = rc; *i; ++i) {
        fs::path newDirectory{ directory / *i };
        if (IsDirectory(newDirectory)) {
            GetFilesFromDirectoryRecursive(newDirectory, results, ext);
        } else {
            fs::path path{ *i };
            if (ext.empty() || ext == path.extension().string()) {
                results[std::move(path)] = directory;
            }
        }
    }

    PHYSFS_freeList(rc);
    return true;
}

std::vector<fs::path> VirtualFileSystem::GetFilesFromDirectory(const fs::path& directory, std::string_view ext) {
    if (!PHYSFS_isInit()) {
        WZ_LOG_FATAL("PHYSFS library was not initialized");
        return {};
    }

    std::vector<fs::path> fileList;

    auto rc = PHYSFS_enumerateFiles(directory.generic_string().c_str());

    for (auto i = rc; *i; ++i) {
        fs::path path{ *i };
        if (!IsDirectory(path)) {
            if (ext.empty() || ext == path.extension().string()) {
                fileList.emplace_back(std::move(path));
            }
        }
    }

    PHYSFS_freeList(rc);
    return fileList;
}