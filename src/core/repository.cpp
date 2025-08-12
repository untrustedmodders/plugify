#include "repository.hpp"

#include <glaze/glaze.hpp>
#include <utils/file_system.hpp>

using namespace plugify;

Result<bool> LocalRepository::IsAvailable() {
	std::error_code ec;
	bool exists = fs::exists(_rootPath, ec);
	if (ec) {
		return Error::FileSystem("check repository path", ec.message());
	}

	bool isDir = fs::is_directory(_rootPath, ec);
	if (ec) {
		return Error::FileSystem("check repository type", ec.message());
	}

	return exists && isDir;
}

std::vector<Package> LocalRepository::GetPackages() override {
	std::vector<Package> packages;

	ScanDirectory(_rootPath, [&](const fs::path& path, int depth) {
		if (depth != 1)
			return;

		if (path.extension() != Manifest::kFileExtension)
			return;

		auto& pkg = packages.emplace_back();
		pkg.location = PackageLocation{PackageLocal{path.parent_path(), path}};

		std::ifstream is(path, std::ios::binary);
		if (!is) {
			pkg.state = PackageState::Corrupted;
			pkg.error = std::format("Failed to open file '{}': {}", path.string(), std::strerror(errno));
			return;
		}

		is.unsetf(std::ios::skipws);

		std::string buffer(std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{});
		auto manifest = glz::read_jsonc<std::shared_ptr<PackageManifest>>(buffer);
		if (!manifest) {
			pkg.state = PackageState::Corrupted;
			pkg.error = std::format("Failed to parse manifest from '{}': {}", path.string(), glz::format_error(manifest.error(), buffer));
			return;
		}

		pkg.manifest = *manifest;

		auto& name = pkg.manifest->name;
		if (name.empty()) {
			name = path.filename().replace_extension().string();
		}
		if (name.empty()) {
			pkg.state = PackageState::Corrupted;
			pkg.error = "Manifest name is empty";
			return;
		}

		pkg.state = PackageState::Installed;
	}, 3);

	return packages; // success
}

template<typename F>
void LocalRepository::ScanDirectory(const fs::path& directory, const F& func, int depth) {
	if (depth <= 0)
		return;

	std::error_code ec;
	for (const auto& entry : fs::directory_iterator(directory, ec)) {
		const auto& path = entry.path();
		if (entry.is_directory(ec)) {
			ScanDirectory(path, func, depth - 1);
		} else if (entry.is_regular_file(ec)) {
			func(path, depth);
		}
	}
}