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
		return ErrorData(assembly->GetError());
	}
	return std::unique_ptr<IAssembly>(std::move(assembly));
}

bool AssemblyLoader::AddSearchPath(fs::path_view path) {
#if PLUGIFY_PLATFORM_WINDOWS
	fs::path libraryDirectory = path;

	std::error_code ec;

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
#endif
	return true;
}