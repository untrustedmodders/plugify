#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <regex>
#include <span>
#include <string>
#include <vector>
#include <optional>

#include "plugify/types.hpp"

namespace plugify {
	/**
	 * @brief File information
	 */
	struct FileInfo {
		std::filesystem::path path;  ///< Path of the entry.
		std::uintmax_t size;  ///< Size in bytes; meaningful for a regular file.
		std::filesystem::file_time_type last_write_time;  ///< When the entry was last written to.
		std::filesystem::file_type type;  ///< What kind of entry this is.
		std::filesystem::perms permissions;  ///< Permission bits of the entry.

		/**
		 * @brief Check whether this entry is a directory
		 * @return true if the entry is a directory
		 */
		bool is_directory() const {
			return type == std::filesystem::file_type::directory;
		}

		/**
		 * @brief Check whether this entry is a regular file
		 * @return true if the entry is a regular file
		 */
		bool is_regular_file() const {
			return type == std::filesystem::file_type::regular;
		}

		/**
		 * @brief Check whether this entry is a symbolic link
		 * @return true if the entry is a symbolic link
		 */
		bool is_symlink() const {
			return type == std::filesystem::file_type::symlink;
		}
	};

	/**
	 * @brief Directory iteration options
	 */
	struct DirectoryIterationOptions {
		bool recursive = false;  ///< Descend into subdirectories.
		bool follow_symlinks = false;  ///< Follow symbolic links while descending.
		bool skip_permission_denied = true;  ///< Skip entries that cannot be read instead of failing.
		std::optional<std::regex> filter_pattern;  ///< Keep only entries whose path matches this pattern.
		std::function<bool(const FileInfo&)> filter_predicate;  ///< Keep only entries this predicate accepts.
	};

	/**
	 * @brief Simple filesystem interface for reading files and iterating directories
	 */
	class IFileSystem {
	public:
		/**
		 * @brief Virtual destructor
		 */
		virtual ~IFileSystem() = default;

		// File Operations
		/**
		 * @brief Read entire file as text
		 * @param path Path to the file
		 * @return The contents of the file or error
		 */
		virtual Result<std::string> ReadTextFile(const std::filesystem::path& path) = 0;

		/**
		 * @brief Read entire file as binary
		 * @param path Path to the file
		 * @return The bytes of the file or error
		 */
		virtual Result<std::vector<uint8_t>> ReadBinaryFile(const std::filesystem::path& path) = 0;

		/**
		 * @brief Write text to file
		 * @param path Path to the file, which is created or truncated
		 * @param content Text to write
		 * @return Success or error
		 */
		virtual Result<void>
		WriteTextFile(const std::filesystem::path& path, std::string_view content) = 0;

		/**
		 * @brief Write binary data to file
		 * @param path Path to the file, which is created or truncated
		 * @param data Bytes to write
		 * @return Success or error
		 */
		virtual Result<void>
		WriteBinaryFile(const std::filesystem::path& path, std::span<const uint8_t> data) = 0;

		// Directory Operations
		/**
		 * @brief Check if path exists
		 * @param path Path to check
		 * @return true if something exists at the path
		 */
		virtual bool IsExists(const std::filesystem::path& path) = 0;

		/**
		 * @brief Check if path is a directory
		 * @param path Path to check
		 * @return true if the path is a directory
		 */
		virtual bool IsDirectory(const std::filesystem::path& path) = 0;

		/**
		 * @brief Check if path is a regular file
		 * @param path Path to check
		 * @return true if the path is a regular file
		 */
		virtual bool IsRegularFile(const std::filesystem::path& path) = 0;

		/**
		 * @brief Get file information
		 * @param path Path to query
		 * @return Information about the entry or error
		 */
		virtual Result<FileInfo> GetFileInfo(const std::filesystem::path& path) = 0;

		/**
		 * @brief List files in directory (non-recursive)
		 * @param directory Directory to list
		 * @return Information about every entry directly inside it, or error
		 */
		virtual Result<std::vector<FileInfo>>
		ListDirectory(const std::filesystem::path& directory) = 0;

		/**
		 * @brief Iterate over files in directory with options
		 * @param directory Directory to walk
		 * @param options How to walk it and which entries to keep
		 * @return Information about the entries that were kept, or error
		 */
		virtual Result<std::vector<FileInfo>> IterateDirectory(
		    const std::filesystem::path& directory,
		    const DirectoryIterationOptions& options = {}
		) = 0;

		/**
		 * @brief Find files matching pattern
		 * @param directory Directory to search in
		 * @param patterns Glob patterns a file has to match one of
		 * @param recursive Whether to search subdirectories as well
		 * @return Paths of the matching files or error
		 */
		virtual Result<std::vector<std::filesystem::path>> FindFiles(
		    const std::filesystem::path& directory,
		    std::span<const std::string_view> patterns,  // e.g., {"*.json", "manifest.*"}
		    bool recursive = true
		) = 0;

		// Path Operations
		/**
		 * @brief Create directory (including parents)
		 * @param path Directory to create
		 * @return Success or error
		 */
		virtual Result<void> CreateDirectories(const std::filesystem::path& path) = 0;

		/**
		 * @brief Remove file or empty directory
		 * @param path Entry to remove
		 * @return Success or error
		 */
		virtual Result<void> Remove(const std::filesystem::path& path) = 0;

		/**
		 * @brief Remove directory and all contents
		 * @param path Directory to remove
		 * @return Success or error
		 */
		virtual Result<void> RemoveAll(const std::filesystem::path& path) = 0;

		/**
		 * @brief Copy file or directory
		 * @param from Entry to copy
		 * @param to Destination path
		 * @return Success or error
		 */
		virtual Result<void>
		Copy(const std::filesystem::path& from, const std::filesystem::path& to) = 0;

		/**
		 * @brief Move/rename file or directory
		 * @param from Entry to move
		 * @param to Destination path
		 * @return Success or error
		 */
		virtual Result<void>
		Move(const std::filesystem::path& from, const std::filesystem::path& to) = 0;

		/**
		 * @brief Make a path absolute, without touching the filesystem
		 * @param path Path to anchor against the current working directory
		 * @return The absolute path or error
		 */
		virtual Result<std::filesystem::path> GetAbsolutePath(const std::filesystem::path& path) = 0;

		/**
		 * @brief Resolve a path to its canonical form
		 * @param path Path to resolve; it has to exist
		 * @return The absolute path with symbolic links and relative steps resolved, or error
		 */
		virtual Result<std::filesystem::path> GetCanonicalPath(const std::filesystem::path& path) = 0;

		/**
		 * @brief Express a path relative to another one
		 * @param path Path to express
		 * @param base Path to express it relative to
		 * @return The relative path or error
		 */
		virtual Result<std::filesystem::path>
		GetRelativePath(const std::filesystem::path& path, const std::filesystem::path& base) = 0;
	};
}  // namespace plugify
