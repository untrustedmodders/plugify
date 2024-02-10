#pragma once

namespace plugify {
	struct LibraryDirectoryHandle;

	class LibraryDirectory {
	public:
		explicit LibraryDirectory(const fs::path& directoryPath);
		LibraryDirectory(LibraryDirectory&& other) noexcept;
		~LibraryDirectory();

	private:
		std::unique_ptr<LibraryDirectoryHandle> _handle;
	};
}
