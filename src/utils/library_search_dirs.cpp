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
			DLL_DIRECTORY_COOKIE cookie = AddDllDirectory(directory.c_str());
			if (cookie == nullptr)
				continue;
			_dirCookies.push_back(cookie);
		}
	}

	~LibrarySearchDirsWin() override {
		for (DLL_DIRECTORY_COOKIE cookie : _dirCookies) {
			RemoveDllDirectory(cookie);
		}
	}

	std::vector<DLL_DIRECTORY_COOKIE> _dirCookies;
};

std::unique_ptr<LibrarySearchDirs> LibrarySearchDirs::Add(const std::vector<fs::path>& additionalSearchDirectories) {
	return std::unique_ptr<LibrarySearchDirs>(new LibrarySearchDirsWin(additionalSearchDirectories));
}

#else

class LibrarySearchDirsDefault final : public LibrarySearchDirs {
public:
	// Cannot set LD_LIBRARY_PATH at runtime, so use rpath flag
	explicit LibrarySearchDirsDefault(const std::vector<fs::path>& /*directories*/) {
	}
	~LibrarySearchDirsDefault() override = default;
};

std::unique_ptr<LibrarySearchDirs> LibrarySearchDirs::Add(const std::vector<fs::path>& additionalSearchDirectories) {
	return std::unique_ptr<LibrarySearchDirs>(new LibrarySearchDirsDefault(additionalSearchDirectories));
}

#endif // PLUGIFY_PLATFORM_WINDOWS
