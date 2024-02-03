#pragma once

namespace plugify {
	struct PackageManifest {
		std::unordered_map<std::string, RemotePackage> content;

		static inline const char* const kFileExtension = ".wpackagemanifest";
	};
	struct VerifiedPackageVersion {
		int32_t version;
		std::string checksum;
		bool operator <(const VerifiedPackageVersion& rhs) const { return version > rhs.version; }
	};
	struct VerifiedPackageDetails {
		std::string name;
		std::string type;
		std::set<VerifiedPackageVersion> versions;
	};
	struct VerifiedPackageMap {
		std::unordered_map<std::string, VerifiedPackageDetails> verified;
	};
}