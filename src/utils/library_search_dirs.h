#pragma once

#include "os.h"

namespace plugify {
	class LibrarySearchDirs {
	public:
		virtual ~LibrarySearchDirs() = 0;

		static std::unique_ptr<LibrarySearchDirs> Add(const std::vector<fs::path>& additionalSearchDirectories);
	};
}
