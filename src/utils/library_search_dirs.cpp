#include "library_search_dirs.h"
#include "platform.h"
#include "os.h"

using namespace plugify;

inline LibrarySearchDirs::~LibrarySearchDirs() = default;

#if PLUGIFY_PLATFORM_WINDOWS

class LibrarySearchDirsWin final : public LibrarySearchDirs {
public:
	explicit LibrarySearchDirsWin(const std::vector<fs::path>& directories) {
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
	explicit LibrarySearchDirsLinux(const std::vector<fs::path>& directories) {
	}
	~LibrarySearchDirsLinux() override = default;
};

std::unique_ptr<LibrarySearchDirs> LibrarySearchDirs::Add(const std::vector<fs::path>& additionalSearchDirectories) {
	return std::unique_ptr<LibrarySearchDirs>(new LibrarySearchDirsLinux(additionalSearchDirectories));
}

#elif PLUGIFY_PLATFORM_APPLE

class LibrarySearchDirsApple final : public LibrarySearchDirs {
public:
	explicit LibrarySearchDirsApple(const std::vector<fs::path>& directories) {
		if (directories.empty())
			return;
		std::string newLibPath;
		for (const auto& directory : directories) {
			std::format_to(std::back_inserter(newLibPath), "{}:", directory.string());
		}
		_curLibPath = GetEnvVariable("DYLD_FALLBACK_LIBRARY_PATH");
		if (_curLibPath.has_value()) {
			std::format_to(std::back_inserter(newLibPath), "{}", *_curLibPath);
		} else {
			if (!newLibPath.empty()) {
				newLibPath.pop_back();
			}
		}
		SetEnvVariable("DYLD_FALLBACK_LIBRARY_PATH", newLibPath.data());
	}

	~LibrarySearchDirsApple() override {
		if(_curLibPath.has_value()) {
			SetEnvVariable("DYLD_FALLBACK_LIBRARY_PATH", _curLibPath->data());
		} else {
			UnsetEnvVariable("DYLD_FALLBACK_LIBRARY_PATH");
		}
	}

	std::optional<std::string> _curLibPath;
};

std::unique_ptr<LibrarySearchDirs> LibrarySearchDirs::Add(const std::vector<fs::path>& additionalSearchDirectories) {
	return std::unique_ptr<LibrarySearchDirs>(new LibrarySearchDirsApple(additionalSearchDirectories));
}

#endif