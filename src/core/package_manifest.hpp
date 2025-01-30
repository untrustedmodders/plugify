#pragma once

#include <plugify/package.hpp>

namespace plugify {
	struct PackageManifest {
		std::unordered_map<std::string, std::shared_ptr<RemotePackage>> content;

		static inline std::string_view kFileExtension = ".pmanifest";
	};
}
