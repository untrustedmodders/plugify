#pragma once

namespace plugify {
	class LibrarySearchDirs {
	public:
		LibrarySearchDirs() = default;
		virtual ~LibrarySearchDirs() = 0;

		static std::unique_ptr<LibrarySearchDirs> Add(const std::vector<fs::path>& additionalSearchDirectories);
	};
}
