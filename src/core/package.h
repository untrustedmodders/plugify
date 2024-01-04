#pragma once

#include <wizard/version.h>

namespace wizard {
	struct Package {
		std::string name;
		std::string url;
		int32_t version;
		bool extractArchive{ false };
		bool languageModule{ false };
	};

	struct PackageManifest {
		std::unordered_map<std::string, Package> content;

		static inline const char* const kFileExtension = ".wpackagemanifest";
	};
}