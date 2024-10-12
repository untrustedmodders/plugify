#pragma once

namespace plugify {
	using FileHandler = std::function<void(std::span<const uint8_t>)>;
	using PathHandler = std::function<void(const fs::path&, int)>;

	class FileSystem {
	public:
		FileSystem() = delete;

		static void ReadBytes(const fs::path& filepath, const FileHandler& handler);

		static std::string ReadText(const fs::path& filepath);

		static bool WriteBytes(const fs::path& filepath, std::span<const uint8_t> buffer);

		static bool WriteText(const fs::path& filepath, std::string_view text);

		static bool IsExists(const fs::path& filepath);

		static bool IsDirectory(const fs::path& filepath);

		static std::vector<fs::path> GetFiles(const fs::path& root, bool recursive = false, std::string_view ext = "");

		static void ReadDirectory(const fs::path& directory, const PathHandler& handler, int depth = 2);

		static std::error_code RemoveFolder(const fs::path& at);

		static std::error_code MoveFolder(const fs::path& from, const fs::path& to);
	};
}
