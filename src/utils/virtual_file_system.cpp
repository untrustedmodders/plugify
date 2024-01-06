#include "virtual_file_system.h"

#include <physfs.h>

using namespace wizard;

void VirtualFileSystem::Initialize(const char* arg0) {
	PHYSFS_init(arg0);
}

void VirtualFileSystem::Shutdown() {
    PHYSFS_deinit();
}

void VirtualFileSystem::Mount(const fs::path& path, const fs::path& mount) {
    PHYSFS_mount(path.generic_string().c_str(), mount.empty() ? nullptr : mount.generic_string().c_str(), 1);
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

bool VirtualFileSystem::GetFiles(const fs::path& directory, std::unordered_map<fs::path, fs::path, PathHash>& results, std::string_view ext) {
    if (!PHYSFS_isInit()) {
        WZ_LOG_FATAL("PHYSFS library was not initialized");
        return false;
    }

    auto rc = PHYSFS_enumerateFiles(directory.generic_string().c_str());

    for (auto i = rc; *i; ++i) {
        fs::path newDirectory{ directory / *i };
        if (IsDirectory(newDirectory)) {
			GetFiles(newDirectory, results, ext);
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

void VirtualFileSystem::ReadDirectory(const fs::path& directory, const PathHandler& handler, int depth) {
	if (depth <= 0)
		return;

	auto rc = PHYSFS_enumerateFiles(directory.generic_string().c_str());

	for (auto i = rc; *i; ++i) {
		fs::path newDirectory{ directory / *i };
		if (IsDirectory(newDirectory)) {
			ReadDirectory(newDirectory, handler, depth - 1);
		} else {
			handler(directory / *i, depth);
		}
	}

	PHYSFS_freeList(rc);
}

#ifdef WIZARD_PLATFORM_WINDOWS
#include <string.h>
#define strcasecmp _stricmp
#else // assuming POSIX or BSD compliant system
#include <strings.h>
#endif

bool VirtualFileSystem::IsArchive(std::string_view extension) {
	if (extension.size() <= 1)
		return false;

	auto rc = PHYSFS_supportedArchiveTypes();
	if (*rc == nullptr)
		return false;

	for (auto i = rc; *i; i++) {
		if (!strcasecmp((*i)->extension, &extension[1]))
			return true;
	}

	return false;
}