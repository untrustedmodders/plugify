#pragma once

namespace plugify {
	struct LibraryDirectoryHandle;

	struct LibraryDirectory {
		explicit LibraryDirectory(fs::path directoryPath);
		LibraryDirectory(LibraryDirectory&& other) noexcept;
		~LibraryDirectory();

		std::unique_ptr<LibraryDirectoryHandle> handle;
	};

	class LibraryDirectories {
	public:
		LibraryDirectories();
		~LibraryDirectories();

		void Add(fs::path directoryPath);
		void Reserve(size_t size);
		void Apply();

	private:
		std::vector<LibraryDirectory> _directories;
		[[maybe_unused]] std::optional<std::string> _curLibPath;
	};
}
