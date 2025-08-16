#include <plugify/asm/assembly_loader.hpp>

#include "defer.hpp"

using namespace plugify;
namespace fs = std::filesystem;

AssemblyResult AssemblyLoader::Load(fs::path_view path, LoadFlag flags) {
	auto assembly = std::make_unique<Assembly>(path, flags, _searchPaths, false);
#if PLUGIFY_PLATFORM_WINDOWS
	defer {
		_searchPaths.clear();
	};
#endif
	if (!assembly->IsValid()) {
		return String(assembly->GetError());
	}
	return std::unique_ptr<IAssembly>(std::move(assembly));
}

bool AssemblyLoader::AddSearchPath(fs::path_view path) {
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

	_searchPaths.emplace_back(std::move(libraryDirectory));

	return true;
#else
	return false;
#endif
}

bool AssemblyLoader::CanLinkSearchPaths() const {
	// Cannot set LD_LIBRARY_PATH at runtime, so use rpath flag
	return PLUGIFY_PLATFORM_WINDOWS;
}
