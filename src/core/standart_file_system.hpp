#pragma once
#include "plugify/core/standart_file_system.hpp"

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

	private:
	    static std::string SystemError();

	    void IterateRecursive(
	        const std::filesystem::path& directory,
	        std::vector<FileInfo>& files,
	        const DirectoryIterationOptions& options,
	        size_t currentDepth,
	        std::error_code& ec);

	    static bool ShouldInclude(const FileInfo& info, const DirectoryIterationOptions& options);
	    static bool MatchesPatterns(std::string_view filename, std::span<const std::string_view> patterns);
	    static bool SimpleMatch(std::string_view text, std::string_view pattern);
	};
} // namespace plugify