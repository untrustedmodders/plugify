#pragma once

namespace wizard {
	struct LibraryDirectoryHandle;

	class LibraryDirectory {
	public:
		LibraryDirectory(const std::filesystem::path& directoryPath);
		LibraryDirectory(LibraryDirectory&& other);
		~LibraryDirectory();

	private:
		std::unique_ptr<LibraryDirectoryHandle> _handle;
	};
}
