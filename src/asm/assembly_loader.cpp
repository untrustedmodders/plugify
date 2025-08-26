#include "plugify/asm/assembly_loader.hpp"

#include "defer.hpp"

using namespace plugify;
namespace fs = std::filesystem;

#if PLUGIFY_PLATFORM_WINDOWS
thread_local std::vector<fs::path> searchPaths;
#endif

Result<std::unique_ptr<IAssembly>> AssemblyLoader::Load(const fs::path& path, LoadFlag flags) {
	auto assembly = std::make_unique<Assembly>(path, flags, searchPaths, false);
#if PLUGIFY_PLATFORM_WINDOWS
	defer {
		searchPaths.clear();
	};
#endif
	if (!assembly->IsValid()) {
		return plg::unexpected(assembly->GetError());
	}
	return std::unique_ptr<IAssembly>(std::move(assembly));
}

bool AssemblyLoader::AddSearchPath(const fs::path& path) {
#if PLUGIFY_PLATFORM_WINDOWS
	std::error_code ec;

	fs::path libraryDirectory = fs::absolute(path, ec);
	if (!fs::is_directory(libraryDirectory, ec)) {
		return false;
	}

	if (fs::is_symlink(libraryDirectory, ec)) {
		libraryDirectory = fs::read_symlink(libraryDirectory, ec);

		if (!fs::is_directory(libraryDirectory, ec)) {
			return false;
		}
	}

	libraryDirectory.make_preferred();

	searchPaths.push_back(std::move(libraryDirectory));

	return true;
#else
	return false;
#endif
}

bool AssemblyLoader::CanLinkSearchPaths() const {
	// Cannot set LD_LIBRARY_PATH at runtime, so use rpath flag
	return PLUGIFY_PLATFORM_WINDOWS;
}
