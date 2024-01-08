#pragma once

#include <wizard/version.h>
#include <wizard/plugin_descriptor.h>
#include <wizard/language_module_descriptor.h>

namespace wizard {
	enum class PackageInstallState : uint8_t {
		None,
		Updating,
		Downloading,
		Checksuming,
		Extracting,
		Done,
		Failed,
	};

	enum class PackageError : uint8_t {
		None,
		FailedReadingArchive,
		FailedCreatingDirectory,
		FailedWritingToDisk,
		PackageMissingDescriptor,
		PackageFetchingFailed,
		PackageAuthorizationFailed,
		PackageCorrupted,
		NoMemoryAvailable,
		NotFound,
	};

	struct PackageVersion {
		int32_t version;
		uint32_t sdkVersion;
		//std::string checksum;
		//std::vector<std::string> platforms;
		std::vector<std::string> mirrors;

		bool operator <(const PackageVersion& rhs) const;
	};

	using PackageRef = std::optional<std::reference_wrapper<const PackageVersion>>;

	// TODO: Add more field ?

	struct RemotePackage {
		std::string name;
		std::string type;
		std::string author;
		std::string description;
		std::set<PackageVersion> versions;

		bool operator==(const RemotePackage& rhs) const;
		PackageRef LatestVersion() const;
		PackageRef Version(int32_t version) const;
	};

	struct LocalPackage {
		std::string name;
		std::string type;
		fs::path path;
		int32_t version;
		std::unique_ptr<Descriptor> descriptor;

		explicit operator RemotePackage() const;
	};

	struct PackageManifest {
		std::unordered_map<std::string, RemotePackage> content;

		static inline const char* const kFileExtension = ".wpackagemanifest";
	};

	struct PackageState {
		size_t progress{};
		size_t total{};
		float ratio{};
		PackageInstallState state{ PackageInstallState::None };
		PackageError error{ PackageError::None };

		std::string_view GetError() const;
		std::string GetProgress(int barWidth = 60) const;
	};

}