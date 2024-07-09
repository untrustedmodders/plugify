#include "library_search_dirs.h"
#include "os.h"

using namespace plugify;

inline LibrarySearchDirs::~LibrarySearchDirs() = default;

#if PLUGIFY_PLATFORM_WINDOWS

class LibrarySearchDirsWin final : public LibrarySearchDirs {
public:
	explicit LibrarySearchDirsWin(const std::vector<fs::path>& directories) {
		_dirCookies.reserve(directories.size());
		for (const auto& directory : directories) {
			auto* cookie = AddDllDirectory(directory.c_str());
			if (cookie == nullptr)
				continue;
			_dirCookies.push_back(cookie);
		}
	}

	~LibrarySearchDirsWin() override {
		for (auto* cookie : _dirCookies) {
			RemoveDllDirectory(cookie);
		}
	}

	std::vector<DLL_DIRECTORY_COOKIE> _dirCookies;
};

std::unique_ptr<LibrarySearchDirs> LibrarySearchDirs::Add(const std::vector<fs::path>& additionalSearchDirectories) {
	return std::unique_ptr<LibrarySearchDirs>(new LibrarySearchDirsWin(additionalSearchDirectories));
}

#elif PLUGIFY_PLATFORM_LINUX

class LibrarySearchDirsLinux final : public LibrarySearchDirs {
public:
	// Cannot set LD_LIBRARY_PATH at runtime, so use rpath flag
	explicit LibrarySearchDirsLinux(const std::vector<fs::path>& /*directories*/) {
	}
	~LibrarySearchDirsLinux() override = default;
};

std::unique_ptr<LibrarySearchDirs> LibrarySearchDirs::Add(const std::vector<fs::path>& additionalSearchDirectories) {
	return std::unique_ptr<LibrarySearchDirs>(new LibrarySearchDirsLinux(additionalSearchDirectories));
}

#elif PLUGIFY_PLATFORM_APPLE

class LibrarySearchDirsApple final : public LibrarySearchDirs {
public:
	// Cannot set DYLD_LIBRARY_PATH at runtime, so use rpath flag
	explicit LibrarySearchDirsApple(const std::vector<fs::path>& /*directories*/) {
	}
	~LibrarySearchDirsApple() override = default;
};

std::unique_ptr<LibrarySearchDirs> LibrarySearchDirs::Add(const std::vector<fs::path>& additionalSearchDirectories) {
	return std::unique_ptr<LibrarySearchDirs>(new LibrarySearchDirsApple(additionalSearchDirectories));
}

#endif
