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

void VirtualFileSystem::Mount(const fs::path& path, const fs::path& mount) {
    PHYSFS_mount(path.string().c_str(), mount.string().c_str(), 1);
}

void VirtualFileSystem::Unmount(const fs::path& path) {
    PHYSFS_unmount(path.string().c_str());
}

void VirtualFileSystem::ReadBytes(const fs::path& filepath, const FileHandler& handler) {
    assert(PHYSFS_isInit());

    auto fsFile = PHYSFS_openRead(filepath.string().c_str());

    if (!fsFile) {
        WIZARD_LOG("Failed to open file: '" + filepath.string() + "' - " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()), ErrorLevel::SEV);
        return;
    }

    auto size = static_cast<size_t>(PHYSFS_fileLength(fsFile));
    std::vector<uint8_t> buffer(size);
    PHYSFS_readBytes(fsFile, buffer.data(), size);

    if (PHYSFS_close(fsFile) == 0) {
        WIZARD_LOG("Failed to close file: '" + filepath.string() + "' - " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()), ErrorLevel::SEV);
    }

    handler(buffer);
}

std::string VirtualFileSystem::ReadText(const fs::path& filepath) {
    assert(PHYSFS_isInit());

    auto fsFile = PHYSFS_openRead(filepath.string().c_str());

    if (!fsFile) {
        WIZARD_LOG("Failed to open file: '" + filepath.string() + "' - " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()), ErrorLevel::SEV);
        return {};
    }

    auto size = static_cast<size_t>(PHYSFS_fileLength(fsFile));
    std::vector<uint8_t> buffer(size);
    PHYSFS_readBytes(fsFile, buffer.data(), size);

    if (PHYSFS_close(fsFile) == 0) {
        WIZARD_LOG("Failed to close file: '" + filepath.string() + "' - " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()), ErrorLevel::SEV);
    }

    return { buffer.begin(), buffer.end() };
}

bool VirtualFileSystem::IsExists(const fs::path& filepath) {
    assert(PHYSFS_isInit());

    return PHYSFS_exists(filepath.string().c_str()) != 0;
}

bool VirtualFileSystem::IsDirectory(const fs::path& filepath) {
    assert(PHYSFS_isInit());

    PHYSFS_Stat stat;
    if (!PHYSFS_stat(filepath.string().c_str(), &stat)) {
        return false;
    } else {
        if (stat.filetype == PHYSFS_FILETYPE_SYMLINK) {
            // PHYSFS_stat() doesn't follow symlinks, so we do it manually
            const char* realdir = PHYSFS_getRealDir(filepath.string().c_str());
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

std::vector<fs::path> VirtualFileSystem::GetFiles(const fs::path& filepath, bool recursive, std::string_view ext) {
    assert(PHYSFS_isInit());

    auto rc = PHYSFS_enumerateFiles(filepath.string().c_str());

    std::vector<fs::path> files;

    for (auto i = rc; *i; ++i) {
        fs::path path { *i };
        if (recursive && IsDirectory(path)) {
            auto filesInFound = GetFiles(path, recursive);
            files.insert(files.end(), filesInFound.begin(), filesInFound.end());
        } else {
            if (ext.empty() || ext == Paths::GetExtension(path))
                files.push_back(std::move(path));
        }
    }

    PHYSFS_freeList(rc);
    return files;
}

std::vector<fs::path> VirtualFileSystem::GetSearchPaths() {
    assert(PHYSFS_isInit());

    auto rc = PHYSFS_getSearchPath();

    std::vector<fs::path> files;

    for (auto i = rc; *i; ++i) {
        files.emplace_back(*i);
    }

    PHYSFS_freeList(rc);
    return files;
}