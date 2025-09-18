#include "core/standart_file_system.hpp"

using namespace plugify;

// Get system error message from errno
std::string StandardFileSystem::GetSystemError(int err) {
	std::string result(1024, '\0');
#if PLUGIFY_PLATFORM_WINDOWS
	strerror_s(result.data(), result.size(), err);
#elif PLUGIFY_PLATFORM_APPLE
	strerror_r(err, result.data(), result.size());
#elif PLUGIFY_PLATFORM_UNIX || PLUGIFY_PLATFORM_LINUX
	// GNU vs POSIX strerror_r
	auto b = result.data();
	auto r = strerror_r(err, b, result.size());
	if (r != b) {
		result.assign(r);
	}
#else
	result.assign(std::strerror(err));
#endif
	result.resize(std::strlen(result.data()));
	return result;
}

// Get detailed error message for stream operations
std::string
StandardFileSystem::GetStreamError(const std::filesystem::path& path, const std::string& operation) {
	int err = errno;  // Capture errno immediately
	if (err != 0) {
		return std::format("{} {}: {}", operation, plg::as_string(path), GetSystemError(err));
	}
	return std::format("{} {}: Unknown error", operation, plg::as_string(path));
}

// File Operations
Result<std::string> StandardFileSystem::ReadTextFile(const std::filesystem::path& path) {
	errno = 0;	// Clear errno before operation
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file) {
		return MakeError(GetStreamError(path, "Failed to open file for reading"));
	}

	// Get file size using seekg
	file.seekg(0, std::ios::end);
	auto size = file.tellg();
	if (size == -1) {
		return MakeError(GetStreamError(path, "Failed to determine file size"));
	}
	file.seekg(0, std::ios::beg);

	// Read content
	std::string content(static_cast<size_t>(size), '\0');
	file.read(content.data(), size);

	if (!file) {
		return MakeError(GetStreamError(path, "Failed to read file content"));
	}

	return content;
}

Result<std::vector<uint8_t>> StandardFileSystem::ReadBinaryFile(const std::filesystem::path& path) {
	errno = 0;	// Clear errno before operation
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file) {
		return MakeError(GetStreamError(path, "Failed to open file for reading"));
	}

	// Get file size
	file.seekg(0, std::ios::end);
	auto size = file.tellg();
	if (size == -1) {
		return MakeError(GetStreamError(path, "Failed to determine file size"));
	}
	file.seekg(0, std::ios::beg);

	// Read content
	std::vector<uint8_t> content(static_cast<size_t>(size));
	file.read(reinterpret_cast<char*>(content.data()), size);

	if (!file) {
		return MakeError(GetStreamError(path, "Failed to read file content"));
	}

	return content;
}

Result<void> StandardFileSystem::WriteTextFile(const std::filesystem::path& path, std::string_view content) {
	// Create parent directories if they don't exist
	auto parent = path.parent_path();
	if (!parent.empty()) {
		std::error_code ec;
		std::filesystem::create_directories(parent, ec);
		if (ec) {
			return MakeError("Failed to create parent directories: {}", ec.message());
		}
	}

	errno = 0;	// Clear errno before operation
	std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file) {
		return MakeError(GetStreamError(path, "Failed to open file for writing"));
	}

	file.write(content.data(), static_cast<std::streamsize>(content.size()));
	file.flush();

	if (!file) {
		return MakeError(GetStreamError(path, "Failed to write to file"));
	}

	return {};
}

Result<void> StandardFileSystem::WriteBinaryFile(
	const std::filesystem::path& path,
	std::span<const uint8_t> data
) {
	// Create parent directories if they don't exist
	auto parent = path.parent_path();
	if (!parent.empty()) {
		std::error_code ec;
		std::filesystem::create_directories(parent, ec);
		if (ec) {
			return MakeError("Failed to create parent directories: {}", ec.message());
		}
	}

	errno = 0;	// Clear errno before operation
	std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file) {
		return MakeError(GetStreamError(path, "Failed to open file for writing"));
	}

	file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
	file.flush();

	if (!file) {
		return MakeError(GetStreamError(path, "Failed to write to file"));
	}

	return {};
}

// Directory Operations
bool StandardFileSystem::IsExists(const std::filesystem::path& path) {
	std::error_code ec;
	return std::filesystem::exists(path, ec);
}

bool StandardFileSystem::IsDirectory(const std::filesystem::path& path) {
	std::error_code ec;
	return std::filesystem::is_directory(path, ec);
}

bool StandardFileSystem::IsRegularFile(const std::filesystem::path& path) {
	std::error_code ec;
	return std::filesystem::is_regular_file(path, ec);
}

Result<FileInfo> StandardFileSystem::GetFileInfo(const std::filesystem::path& path) {
	std::error_code ec;
	auto status = std::filesystem::status(path, ec);
	if (ec) {
		return MakeError("Failed to get file status: {} - {}", plg::as_string(path), ec.message());
	}

	FileInfo info;
	info.path = path;
	info.type = status.type();
	info.permissions = status.permissions();

	if (is_regular_file(status)) {
		info.size = std::filesystem::file_size(path, ec);
		if (ec) {
			info.size = 0;	// Set to 0 if we can't get size
		}
	} else {
		info.size = 0;
	}

	info.last_write_time = std::filesystem::last_write_time(path, ec);
	if (ec) {
		info.last_write_time = std::filesystem::file_time_type{};
	}

	return info;
}

Result<std::vector<FileInfo>> StandardFileSystem::ListDirectory(const std::filesystem::path& directory) {
	if (!IsDirectory(directory)) {
		return MakeError("Path is not a directory: {}", plg::as_string(directory));
	}

	std::vector<FileInfo> files;
	std::error_code ec;

	for (auto it = std::filesystem::directory_iterator(directory, ec);
		 it != std::filesystem::directory_iterator();) {
		if (ec) {
			if (ec == std::errc::permission_denied) {
				++it;  // Skip permission denied entries
				ec.clear();
				continue;
			}
			return MakeError("Error iterating directory: " + ec.message());
		}

		auto info_result = GetFileInfo(it->path());
		if (info_result) {
			files.push_back(std::move(*info_result));
		}

		it.increment(ec);
	}

	return files;
}

Result<std::vector<FileInfo>> StandardFileSystem::IterateDirectory(
	const std::filesystem::path& directory,
	const DirectoryIterationOptions& options
) {
	if (!IsDirectory(directory)) {
		return MakeError("Path is not a directory: {}", plg::as_string(directory));
	}

	std::vector<FileInfo> files;
	std::error_code ec;

	auto process_entry = [&](const std::filesystem::directory_entry& entry) {
		auto info_result = GetFileInfo(entry.path());
		if (!info_result) {
			return;
		}

		FileInfo& info = *info_result;

		// Apply filter pattern if specified
		if (options.filter_pattern.has_value()) {
			if (!std::regex_match(plg::as_string(info.path.filename()), options.filter_pattern.value())) {
				return;
			}
		}

		// Apply filter predicate if specified
		if (options.filter_predicate) {
			if (!options.filter_predicate(info)) {
				return;
			}
		}

		files.push_back(std::move(info));
	};

	if (options.recursive) {
		auto dir_options = options.follow_symlinks
							   ? std::filesystem::directory_options::follow_directory_symlink
							   : std::filesystem::directory_options::none;

		for (auto it = std::filesystem::recursive_directory_iterator(directory, dir_options, ec);
			 it != std::filesystem::recursive_directory_iterator();) {
			if (ec) {
				if (options.skip_permission_denied && ec == std::errc::permission_denied) {
					it.disable_recursion_pending();
					++it;
					ec.clear();
					continue;
				}
				return MakeError("Error iterating directory recursively: {}", ec.message());
			}

			process_entry(*it);
			it.increment(ec);
		}
	} else {
		for (auto it = std::filesystem::directory_iterator(directory, ec);
			 it != std::filesystem::directory_iterator();) {
			if (ec) {
				if (options.skip_permission_denied && ec == std::errc::permission_denied) {
					++it;
					ec.clear();
					continue;
				}
				return MakeError("Error iterating directory: {}", ec.message());
			}

			process_entry(*it);
			it.increment(ec);
		}
	}

	return files;
}

Result<std::vector<std::filesystem::path>> StandardFileSystem::FindFiles(
	const std::filesystem::path& directory,
	std::span<const std::string_view> patterns,
	bool recursive
) {
	if (!IsDirectory(directory)) {
		return MakeError("Path is not a directory: {}", plg::as_string(directory));
	}

	// Convert glob patterns to regex
	std::vector<std::regex> regexes;
	regexes.reserve(patterns.size());
	for (const auto& pattern : patterns) {
		std::string regex_str;
		for (char c : pattern) {
			switch (c) {
				case '*':
					regex_str += ".*";
					break;
				case '?':
					regex_str += ".";
					break;
				case '.':
					regex_str += "\\.";
					break;
				case '\\':
					regex_str += "\\\\";
					break;
				case '[':
					regex_str += "[";
					break;
				case ']':
					regex_str += "]";
					break;
				case '^':
					regex_str += "\\^";
					break;
				case '$':
					regex_str += "\\$";
					break;
				case '+':
					regex_str += "\\+";
					break;
				case '|':
					regex_str += "\\|";
					break;
				case '(':
					regex_str += "\\(";
					break;
				case ')':
					regex_str += "\\)";
					break;
				case '{':
					regex_str += "\\{";
					break;
				case '}':
					regex_str += "\\}";
					break;
				default:
					regex_str += c;
					break;
			}
		}
		regexes.emplace_back(regex_str, std::regex_constants::icase);
	}

	std::vector<std::filesystem::path> matches;
	std::error_code ec;

	auto check_file = [&](const std::filesystem::path& file_path) {
		if (!std::filesystem::is_regular_file(file_path, ec)) {
			return;
		}

		std::string filename = plg::as_string(file_path.filename());
		for (const auto& regex : regexes) {
			if (std::regex_match(filename, regex)) {
				matches.push_back(file_path);
				break;
			}
		}
	};

	if (recursive) {
		for (auto it = std::filesystem::recursive_directory_iterator(
				 directory,
				 std::filesystem::directory_options::skip_permission_denied,
				 ec
			 );
			 it != std::filesystem::recursive_directory_iterator();) {
			if (!ec) {
				check_file(it->path());
			}
			it.increment(ec);
		}
	} else {
		for (auto it = std::filesystem::directory_iterator(directory, ec);
			 it != std::filesystem::directory_iterator();) {
			if (!ec) {
				check_file(it->path());
			}
			it.increment(ec);
		}
	}

	return matches;
}

// Path Operations
Result<void> StandardFileSystem::CreateDirectories(const std::filesystem::path& path) {
	std::error_code ec;
	std::filesystem::create_directories(path, ec);
	if (ec) {
		return MakeError("Failed to create directories: {} - {}", plg::as_string(path), ec.message());
	}
	return {};
}

Result<void> StandardFileSystem::Remove(const std::filesystem::path& path) {
	std::error_code ec;
	std::filesystem::remove(path, ec);
	if (ec && ec != std::errc::no_such_file_or_directory) {
		return MakeError("Failed to remove: {} - {}", plg::as_string(path), ec.message());
	}
	return {};
}

Result<void> StandardFileSystem::RemoveAll(const std::filesystem::path& path) {
	std::error_code ec;
	std::filesystem::remove_all(path, ec);
	if (ec && ec != std::errc::no_such_file_or_directory) {
		return MakeError("Failed to remove all: {} - {}", plg::as_string(path), ec.message());
	}
	return {};
}

Result<void> StandardFileSystem::Copy(const std::filesystem::path& from, const std::filesystem::path& to) {
	// Create parent directory if it doesn't exist
	auto parent = to.parent_path();
	if (!parent.empty()) {
		std::error_code ec;
		std::filesystem::create_directories(parent, ec);
		if (ec) {
			return MakeError("Failed to create parent directories: {}", ec.message());
		}
	}

	std::error_code ec;
	std::filesystem::copy(
		from,
		to,
		std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing,
		ec
	);

	if (ec) {
		return MakeError(
			"Failed to copy from {} to {} - {}",
			plg::as_string(from),
			plg::as_string(to),
			ec.message()
		);
	}
	return {};
}

Result<void> StandardFileSystem::Move(const std::filesystem::path& from, const std::filesystem::path& to) {
	// Create parent directory if it doesn't exist
	auto parent = to.parent_path();
	if (!parent.empty()) {
		std::error_code ec;
		std::filesystem::create_directories(parent, ec);
		if (ec) {
			return MakeError("Failed to create parent directories: {}", ec.message());
		}
	}

	std::error_code ec;
	std::filesystem::rename(from, to, ec);
	if (ec) {
		// If rename fails (e.g., across filesystems), try copy and remove
		ec.clear();
		std::filesystem::copy(
			from,
			to,
			std::filesystem::copy_options::recursive
				| std::filesystem::copy_options::overwrite_existing,
			ec
		);
		if (ec) {
			return MakeError(
				"Failed to move from {} to {} - {}",
				plg::as_string(from),
				plg::as_string(to),
				ec.message()
			);
		}

		std::filesystem::remove_all(from, ec);
		if (ec) {
			// Copy succeeded but remove failed - file is moved but source remains
			return MakeError("Warning: moved file but failed to remove source: {}", ec.message());
		}
	}
	return {};
}

// Get canonical path (resolves symlinks and relative paths)
Result<std::filesystem::path> StandardFileSystem::GetCanonicalPath(const std::filesystem::path& path) {
	std::error_code ec;
	auto canonical = std::filesystem::canonical(path, ec);
	if (ec) {
		return MakeError("Failed to get canonical path: {}", ec.message());
	}
	return canonical;
}

// Get absolute path
Result<std::filesystem::path> StandardFileSystem::GetAbsolutePath(const std::filesystem::path& path) {
	std::error_code ec;
	auto absolute = std::filesystem::absolute(path, ec);
	if (ec) {
		return MakeError("Failed to get absolute path: {}", ec.message());
	}
	return absolute;
}

// Get relative path
Result<std::filesystem::path> StandardFileSystem::GetRelativePath(
	const std::filesystem::path& path,
	const std::filesystem::path& base
) {
	std::error_code ec;
	auto relative = std::filesystem::relative(path, base, ec);
	if (ec) {
		return MakeError(
			"Failed to relative path to {} with {} - {}",
			plg::as_string(path),
			plg::as_string(base),
			ec.message()
		);
	}
	return relative;
}

// Append operations
Result<void>
ExtendedFileSystem::AppendTextFile(const std::filesystem::path& path, std::string_view content) {
	errno = 0;	// Clear errno before operation
	std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::app);
	if (!file) {
		return MakeError(GetStreamError(path, "Failed to open file for appending"));
	}

	file.write(content.data(), static_cast<std::streamsize>(content.size()));
	file.flush();

	if (!file) {
		return MakeError(GetStreamError(path, "Failed to append to file"));
	}

	return {};
}

Result<void> ExtendedFileSystem::AppendBinaryFile(
	const std::filesystem::path& path,
	std::span<const uint8_t> data
) {
	errno = 0;	// Clear errno before operation
	std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::app);
	if (!file) {
		return MakeError(GetStreamError(path, "Failed to open file for appending"));
	}

	file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
	file.flush();

	if (!file) {
		return MakeError(GetStreamError(path, "Failed to append to file"));
	}

	return {};
}

// Atomic write operation
Result<void>
ExtendedFileSystem::WriteFileAtomic(const std::filesystem::path& path, std::string_view content) {
	// Generate temporary file path
	auto temp_path = path;
	temp_path += std::format(".tmp{}", std::chrono::steady_clock::now().time_since_epoch().count());

	// Write to temporary file
	auto write_result = WriteTextFile(temp_path, content);
	if (!write_result) {
		return write_result;
	}

	// Atomically rename temp file to target
	std::error_code ec;
	std::filesystem::rename(temp_path, path, ec);
	if (ec) {
		// Clean up temp file
		std::filesystem::remove(temp_path, ec);
		return MakeError("Failed to atomically replace file: {}", ec.message());
	}

	return {};
}

// Get available space
Result<std::filesystem::space_info> ExtendedFileSystem::GetSpaceInfo(const std::filesystem::path& path) {
	std::error_code ec;
	auto space = std::filesystem::space(path, ec);
	if (ec) {
		return MakeError("Failed to get space info: {}", ec.message());
	}
	return space;
}

// Create temporary file
Result<std::filesystem::path> ExtendedFileSystem::CreateTempFile(
	const std::filesystem::path& directory,
	const std::string& prefix
) {
	std::error_code ec;
	std::filesystem::path dir = directory.empty() ? std::filesystem::temp_directory_path(ec)
												  : directory;

	if (ec) {
		return MakeError("Failed to get temp directory: {}", ec.message());
	}

	// Generate unique filename
	auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
	std::filesystem::path temp_path = dir / std::format("{}_{}", prefix, timestamp);

	// Create empty file
	errno = 0;	// Clear errno before operation
	std::ofstream file(temp_path);
	if (!file) {
		return MakeError(GetStreamError(temp_path, "Failed to create temp file"));
	}
	file.close();

	return temp_path;
}

// Create temporary directory
Result<std::filesystem::path> ExtendedFileSystem::CreateTempDirectory(
	const std::filesystem::path& directory,
	const std::string& prefix
) {
	std::error_code ec;
	std::filesystem::path dir = directory.empty() ? std::filesystem::temp_directory_path(ec)
												  : directory;

	if (ec) {
		return MakeError("Failed to get temp directory: {}", ec.message());
	}

	// Generate unique directory name
	auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
	std::filesystem::path temp_path = dir / std::format("{}_{}", prefix, timestamp);

	// Create directory
	std::filesystem::create_directories(temp_path, ec);
	if (ec) {
		return MakeError("Failed to create temp directory: {}", ec.message());
	}

	return temp_path;
}

// Check if files are equal
Result<bool> ExtendedFileSystem::FilesEqual(
	const std::filesystem::path& path1,
	const std::filesystem::path& path2
) {
	// Quick check: file sizes
	std::error_code ec;
	auto size1 = std::filesystem::file_size(path1, ec);
	if (ec) {
		return MakeError("Failed to get size of first file: {}", ec.message());
	}

	auto size2 = std::filesystem::file_size(path2, ec);
	if (ec) {
		return MakeError("Failed to get size of second file: {}", ec.message());
	}

	if (size1 != size2) {
		return false;
	}

	// Compare contents
	errno = 0;
	std::ifstream file1(path1, std::ios::binary);
	std::ifstream file2(path2, std::ios::binary);

	if (!file1 || !file2) {
		std::string error = "Failed to open files for comparison";
		int err = errno;
		if (err != 0) {
			error = std::format("{}: {}", error, GetSystemError(err));
		}
		return MakeError(error);
	}

	constexpr size_t buffer_size = 8192;
	std::vector<char> buffer1(buffer_size);
	std::vector<char> buffer2(buffer_size);

	while (file1 && file2) {
		file1.read(buffer1.data(), buffer_size);
		file2.read(buffer2.data(), buffer_size);

		auto read1 = file1.gcount();
		auto read2 = file2.gcount();

		if (read1 != read2) {
			return false;
		}

		if (std::memcmp(buffer1.data(), buffer2.data(), static_cast<size_t>(read1)) != 0) {
			return false;
		}

		if (read1 < static_cast<std::streamsize>(buffer_size)) {
			break;	// Reached end of file
		}
	}

	return true;
}

// Compute file hash (simple implementation using std::hash)
Result<size_t> ExtendedFileSystem::ComputeSimpleHash(const std::filesystem::path& path) {
	auto content = ReadBinaryFile(path);
	if (!content) {
		return MakeError("Failed to read file for hashing: {}", content.error());
	}

	std::hash<std::string_view> hasher;
	std::string_view view(reinterpret_cast<const char*>(content->data()), content->size());
	return hasher(view);
}

// Get file permissions in a more detailed format
Result<std::filesystem::perms> ExtendedFileSystem::GetPermissions(const std::filesystem::path& path) {
	std::error_code ec;
	auto status = std::filesystem::status(path, ec);
	if (ec) {
		return MakeError("Failed to get file permissions: {}", ec.message());
	}
	return status.permissions();
}

// Set file permissions
Result<void>
ExtendedFileSystem::SetPermissions(const std::filesystem::path& path, std::filesystem::perms perms) {
	std::error_code ec;
	std::filesystem::permissions(path, perms, ec);
	if (ec) {
		return MakeError("Failed to set file permissions: {}", ec.message());
	}
	return {};
}

// Check if path is writable
bool ExtendedFileSystem::IsWritable(const std::filesystem::path& path) {
	std::error_code ec;
	auto status = std::filesystem::status(path, ec);
	if (ec) {
		return false;
	}

	auto perms = status.permissions();
	return (perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none;
}

// Check if path is readable
bool ExtendedFileSystem::IsReadable(const std::filesystem::path& path) {
	std::error_code ec;
	auto status = std::filesystem::status(path, ec);
	if (ec) {
		return false;
	}

	auto perms = status.permissions();
	return (perms & std::filesystem::perms::owner_read) != std::filesystem::perms::none;
}

// Get last write time
Result<std::filesystem::file_time_type> ExtendedFileSystem::GetLastWriteTime(
	const std::filesystem::path& path
) {
	std::error_code ec;
	auto time = std::filesystem::last_write_time(path, ec);
	if (ec) {
		return MakeError("Failed to get last write time: {}", ec.message());
	}
	return time;
}

// Set last write time
Result<void> ExtendedFileSystem::SetLastWriteTime(
	const std::filesystem::path& path,
	std::filesystem::file_time_type time
) {
	std::error_code ec;
	std::filesystem::last_write_time(path, time, ec);
	if (ec) {
		return MakeError("Failed to set last write time: {}", ec.message());
	}
	return {};
}

// Check if path is a symbolic link
bool ExtendedFileSystem::IsSymlink(const std::filesystem::path& path) {
	std::error_code ec;
	return std::filesystem::is_symlink(path, ec);
}

// Create symbolic link
Result<void> ExtendedFileSystem::CreateSymlink(
	const std::filesystem::path& target,
	const std::filesystem::path& link
) {
	std::error_code ec;
	std::filesystem::create_symlink(target, link, ec);
	if (ec) {
		return MakeError("Failed to create symbolic link: {}", ec.message());
	}
	return {};
}

// Read symbolic link target
Result<std::filesystem::path> ExtendedFileSystem::ReadSymlink(const std::filesystem::path& path) {
	std::error_code ec;
	auto target = std::filesystem::read_symlink(path, ec);
	if (ec) {
		return MakeError("Failed to read symbolic link: {}", ec.message());
	}
	return target;
}