#pragma once

#include <plugify/package.hpp>

namespace plugify {
	struct PackageManifest final {
		std::unordered_map<std::string, RemotePackage> content;

		static inline const char* const kFileExtension = ".pmanifest";
	};
}
