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
	explicit LibrarySearchDirsLinux(const std::vector<fs::path>& directories) {
		if (directories.empty())
			return;
		_curLibPath = GetEnvVariable("LD_LIBRARY_PATH");
		if (_curLibPath.has_value()) {
			std::string newLibPath;
			for (const auto& directory : directories) {
				std::format_to(std::back_inserter(newLibPath), "{}:", directory.string());
			}
			std::format_to(std::back_inserter(newLibPath), "{}", *_curLibPath);
			SetEnvVariable("LD_LIBRARY_PATH", newLibPath);
		}
	}

	~LibrarySearchDirsLinux() override {
		if(_curLibPath.has_value()) {
			SetEnvVariable("LD_LIBRARY_PATH", *_curLibPath);
		}
	}

	std::optional<std::string> _curLibPath;
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
		_curLibPath = GetEnvVariable("DYLD_LIBRARY_PATH");
		if (_curLibPath.has_value()) {
			std::string newLibPath;
			for (const auto& directory : directories) {
				std::format_to(std::back_inserter(newLibPath), "{}:", directory.string());
			}
			std::format_to(std::back_inserter(newLibPath), "{}", *_curLibPath);
			SetEnvVariable("DYLD_LIBRARY_PATH", newLibPath);
		}
	}

	~LibrarySearchDirsApple() override {
		if(_curLibPath.has_value()) {
			SetEnvVariable("DYLD_LIBRARY_PATH", *_curLibPath);
		}
	}

	std::optional<std::string> _curLibPath;
};

std::unique_ptr<LibrarySearchDirs> LibrarySearchDirs::Add(const std::vector<fs::path>& additionalSearchDirectories) {
	return std::unique_ptr<LibrarySearchDirs>(new LibrarySearchDirsApple(additionalSearchDirectories));
}

#endif