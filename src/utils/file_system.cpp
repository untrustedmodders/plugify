#include "file_system.hpp"

#include <fstream>

using namespace plugify;

void FileSystem::ReadBytes(const fs::path& filepath, const FileHandler& handler) {
	std::ifstream is(filepath, std::ios::binary);

	 if (!is.is_open()) {
		PL_LOG_ERROR("File: '{}' could not be opened", filepath.string());
		return;
	 }

	 // Stop eating new lines in binary mode!!!
	 is.unsetf(std::ios::skipws);

	 std::vector<uint8_t> buffer{ std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{} };
	 handler({ buffer.data(), buffer.size() });
}

std::string FileSystem::ReadText(const fs::path& filepath) {
	std::ifstream is(filepath, std::ios::binary);

	if (!is.is_open()) {
		PL_LOG_ERROR("File: '{}' could not be opened", filepath.string());
		return {};
	}

	// Stop eating new lines in binary mode!!!
	is.unsetf(std::ios::skipws);

	return { std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{} };
}

bool FileSystem::WriteBytes(const fs::path& filepath, std::span<const uint8_t> buffer) {
	std::ofstream os(filepath, std::ios::binary);
	os.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
	return static_cast<bool>(os);
}

bool FileSystem::WriteText(const fs::path& filepath, std::string_view text) {
	std::ofstream os(filepath, std::ios::binary);
	os.write(text.data(), static_cast<std::streamsize>(text.size()));
	return static_cast<bool>(os);
}

bool FileSystem::IsExists(const fs::path& filepath) {
	std::error_code ec;
	return fs::exists(filepath, ec);
}

bool FileSystem::IsDirectory(const fs::path& filepath) {
	std::error_code ec;
	return fs::is_directory(filepath, ec);
}

std::vector<fs::path> FileSystem::GetFiles(const fs::path& root, bool recursive, std::string_view ext) {
	std::vector<fs::path> paths;

	std::error_code ec;
	if (fs::is_directory(root, ec)) {
		auto iterate = [&](auto iterator) {
			for (auto const& entry : iterator) {
				const auto& path = entry.path();
				if (entry.is_regular_file(ec) && (ext.empty() || path.extension() == ext)) {
					paths.push_back(path);
				}
			}
		};

		if (recursive) {
			iterate(fs::recursive_directory_iterator(root, ec));
		} else {
			iterate(fs::directory_iterator(root, ec));
		}
	}

	return paths;
}

void FileSystem::ReadDirectory(const fs::path& directory, const PathHandler& handler, int depth) {
	if (depth <= 0)
		return;

	std::error_code ec;
	for (const auto& entry : fs::directory_iterator(directory, ec)) {
		const auto& path = entry.path();
		if (entry.is_directory(ec)) {
			ReadDirectory(path, handler, depth - 1);
		} else if (entry.is_regular_file(ec)) {
			handler(path, depth);
		}
	}
}

std::error_code FileSystem::MoveFolder(const fs::path& from, const fs::path& to) {
	std::error_code ec = RemoveFolder(to);
	if (!ec) {
		fs::rename(from, to, ec);
	}
	return ec;
}

std::error_code FileSystem::RemoveFolder(const fs::path& at) {
	std::error_code ec;
	if (fs::is_directory(at, ec)) {
		fs::remove_all(at, ec);
	}
	return ec;
}
