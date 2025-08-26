#pragma once

#include <span>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <functional>

#include "plugify/core/types.hpp"

namespace plugify {
	/**
	 * @brief File information
	 */
	struct FileInfo {
	    std::filesystem::path path;
	    std::filesystem::file_time_type lastModified;
	    std::uintmax_t size{};
	    bool isDirectory{};
	    bool isRegularFile{};
	    bool isSymlink{};
	};

	/**
	 * @brief Directory iteration options
	 */
	struct DirectoryIterationOptions {
	    bool recursive{true};
	    bool followSymlinks{false};
	    std::optional<size_t> maxDepth;
	    std::vector<std::string> extensions;  // Filter by extensions (e.g., {".json", ".yaml"})
	    std::function<bool(const FileInfo&)> filter;  // Custom filter predicate
	};

	/**
	 * @brief Simple filesystem interface for reading files and iterating directories
	 */
	class IFileSystem {
	public:
	    virtual ~IFileSystem() = default;

	    // File Operations
	    /**
	     * @brief Read entire file as text
	     */
	    virtual Result<std::string> ReadTextFile(const std::filesystem::path& path) = 0;

	    /**
	     * @brief Read entire file as binary
	     */
	    virtual Result<std::vector<uint8_t>> ReadBinaryFile(const std::filesystem::path& path) = 0;

	    /**
	     * @brief Write text to file
	     */
	    virtual Result<void> WriteTextFile(const std::filesystem::path& path, std::string_view content) = 0;

	    /**
	     * @brief Write binary data to file
	     */
	    virtual Result<void> WriteBinaryFile(const std::filesystem::path& path, std::span<const uint8_t> data) = 0;

	    // Directory Operations
	    /**
	     * @brief Check if path exists
	     */
	    virtual bool IsExists(const std::filesystem::path& path) = 0;

	    /**
	     * @brief Check if path is a directory
	     */
	    virtual bool IsDirectory(const std::filesystem::path& path) = 0;

	    /**
	     * @brief Check if path is a regular file
	     */
	    virtual bool IsRegularFile(const std::filesystem::path& path) = 0;

	    /**
	     * @brief Get file information
	     */
	    virtual Result<FileInfo> GetFileInfo(const std::filesystem::path& path) = 0;

	    /**
	     * @brief List files in directory (non-recursive)
	     */
	    virtual Result<std::vector<FileInfo>> ListDirectory(const std::filesystem::path& directory) = 0;

	    /**
	     * @brief Iterate over files in directory with options
	     */
	    virtual Result<std::vector<FileInfo>> IterateDirectory(
	        const std::filesystem::path& directory,
	        const DirectoryIterationOptions& options = {}) = 0;

	    /**
	     * @brief Find files matching pattern
	     */
	    virtual Result<std::vector<std::filesystem::path>> FindFiles(
	        const std::filesystem::path& directory,
	        std::span<const std::string_view> patterns,  // e.g., {"*.json", "manifest.*"}
	        bool recursive = true) = 0;

	    // Path Operations
	    /**
	     * @brief Create directory (including parents)
	     */
	    virtual Result<void> CreateDirectories(const std::filesystem::path& path) = 0;

	    /**
	     * @brief Remove file or empty directory
	     */
	    virtual Result<void> Remove(const std::filesystem::path& path) = 0;

	    /**
	     * @brief Remove directory and all contents
	     */
	    virtual Result<void> RemoveAll(const std::filesystem::path& path) = 0;

	    /**
	     * @brief Copy file or directory
	     */
	    virtual Result<void> Copy(const std::filesystem::path& from, const std::filesystem::path& to) = 0;

	    /**
	     * @brief Move/rename file or directory
	     */
	    virtual Result<void> Move(const std::filesystem::path& from, const std::filesystem::path& to) = 0;
	};
} // namespace plugify
