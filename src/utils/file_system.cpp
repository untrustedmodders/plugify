#include "file_system.h"

#include <fstream>

using namespace wizard;

void FileSystem::ReadBytes(const fs::path& /*filepath*/, const FileHandler& /*handler*/) {
	// std::basic_ifstream<uint8_t, std::char_traits<uint8_t>> is{filepath, std::ios::binary};
	// 
	// if (!is.is_open()) {
	//	 WZ_LOG_ERROR("File: '{}' could not be opened", filepath.string());
	//	 return;
	// }
	// 
	// // Stop eating new lines in binary mode!!!
	// is.unsetf(std::ios::skipws);
	// 
	// std::vector<uint8_t> buffer{ std::istreambuf_iterator<uint8_t>{is}, std::istreambuf_iterator<uint8_t>{} };
	// handler({ buffer.data(), buffer.size() });
}

std::string FileSystem::ReadText(const fs::path& filepath) {
	std::ifstream is{filepath, std::ios::binary};

	if (!is.is_open()) {
		WZ_LOG_ERROR("File: '{}' could not be opened", filepath.string());
		return {};
	}

	// Stop eating new lines in binary mode!!!
	is.unsetf(std::ios::skipws);

	return { std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{} };
}

bool FileSystem::WriteBytes(const fs::path& filepath, std::span<const uint8_t> buffer) {
	std::ofstream os{filepath, std::ios::binary};
	os.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
	return true;
}

bool FileSystem::WriteText(const fs::path& filepath, std::string_view text) {
	std::ofstream os{filepath, std::ios::binary};
	os.write(text.data(), static_cast<std::streamsize>(text.size()));
	return true;
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
	if (fs::exists(root, ec) && fs::is_directory(root, ec)) {
		auto iterate = [&](auto iterator) {
			for (auto const& entry : iterator) {
				const auto& path = entry.path();
				if (fs::is_regular_file(entry, ec) && (ext.empty() || path.extension().string() == ext)) {
					paths.push_back(path);
				}
			}
		};

		if (recursive) {
			iterate(fs::recursive_directory_iterator(root));
		} else {
			iterate(fs::directory_iterator(root));
		}
	}

	return paths;
}