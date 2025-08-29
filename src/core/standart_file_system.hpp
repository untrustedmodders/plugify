#pragma once
#include "plugify/core/file_system.hpp"

namespace plugify {

	// ============================================================================
	// Standard FileSystem Implementation
	// ============================================================================

	/**
	 * @brief Standard implementation using std::filesystem
	 */
	class StandardFileSystem : public IFileSystem {
	public:
	    Result<std::string> ReadTextFile(const std::filesystem::path& path) override;
	    Result<std::vector<uint8_t>> ReadBinaryFile(const std::filesystem::path& path) override;
	    Result<void> WriteTextFile(const std::filesystem::path& path, std::string_view content) override;
	    Result<void> WriteBinaryFile(const std::filesystem::path& path, std::span<const uint8_t> data) override;

	    bool IsExists(const std::filesystem::path& path) override;
	    bool IsDirectory(const std::filesystem::path& path) override;
	    bool IsRegularFile(const std::filesystem::path& path) override;

	    Result<FileInfo> GetFileInfo(const std::filesystem::path& path) override;
	    Result<std::vector<FileInfo>> ListDirectory(const std::filesystem::path& directory) override;
	    Result<std::vector<FileInfo>> IterateDirectory(
	        const std::filesystem::path& directory,
	        const DirectoryIterationOptions& options) override;

	    Result<std::vector<std::filesystem::path>> FindFiles(
	        const std::filesystem::path& directory,
	        std::span<const std::string_view> patterns,
	        bool recursive) override;

	    Result<void> CreateDirectories(const std::filesystem::path& path) override;
	    Result<void> Remove(const std::filesystem::path& path) override;
	    Result<void> RemoveAll(const std::filesystem::path& path) override;
	    Result<void> Copy(const std::filesystem::path& from, const std::filesystem::path& to) override;
	    Result<void> Move(const std::filesystem::path& from, const std::filesystem::path& to) override;

	    Result<std::filesystem::path> GetAbsolutePath(const std::filesystem::path& path) override;
	    Result<std::filesystem::path> GetCanonicalPath(const std::filesystem::path& path) override;
	    Result<std::filesystem::path> GetRelativePath(const std::filesystem::path& path, const std::filesystem::path& base) override;

	protected:
	    static std::string GetSystemError(int err);
	    static std::string GetStreamError(const std::filesystem::path& path, const std::string& operation);
	};

    // Extended functionality implementation
    class ExtendedFileSystem : public StandardFileSystem {
    public:
        // Append operations
        Result<void> AppendTextFile(const std::filesystem::path& path, std::string_view content);
        Result<void> AppendBinaryFile(const std::filesystem::path& path, std::span<const uint8_t> data);
        // Atomic write operation
        Result<void> WriteFileAtomic(const std::filesystem::path& path, std::string_view content);
        // Get available space
        Result<std::filesystem::space_info> GetSpaceInfo(const std::filesystem::path& path);
        // Create temporary file
        Result<std::filesystem::path> CreateTempFile(const std::filesystem::path& directory = {},
                                                     const std::string& prefix = "tmp");

        // Create temporary directory
        Result<std::filesystem::path> CreateTempDirectory(const std::filesystem::path& directory = {},
                                                          const std::string& prefix = "tmpdir");
        // Check if files are equal
        Result<bool> FilesEqual(const std::filesystem::path& path1, const std::filesystem::path& path2);
        // Compute file hash (simple implementation using std::hash)
        Result<size_t> ComputeSimpleHash(const std::filesystem::path& path);
        // Get file permissions in a more detailed format
        Result<std::filesystem::perms> GetPermissions(const std::filesystem::path& path);
        // Set file permissions
        Result<void> SetPermissions(const std::filesystem::path& path, std::filesystem::perms perms);
        // Check if path is writable
        bool IsWritable(const std::filesystem::path& path);
        // Check if path is readable
        bool IsReadable(const std::filesystem::path& path);
        // Get last write time
        Result<std::filesystem::file_time_type> GetLastWriteTime(const std::filesystem::path& path);
        // Set last write time
        Result<void> SetLastWriteTime(const std::filesystem::path& path,
                                      std::filesystem::file_time_type time);
        // Check if path is a symbolic link
        bool IsSymlink(const std::filesystem::path& path);
        // Create symbolic link
        Result<void> CreateSymlink(const std::filesystem::path& target, const std::filesystem::path& link);
        // Read symbolic link target
        Result<std::filesystem::path> ReadSymlink(const std::filesystem::path& path);
    };
} // namespace plugify