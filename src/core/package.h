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
		InvalidURL,
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

	struct RemotePackage {
		std::string name;
		std::string url;
		int32_t version;
		bool module{ false };
	};

	struct LocalPackage {
		std::string name;
		fs::path path;
		int32_t version;
		bool module{ false };
		std::unique_ptr<Descriptor> descriptor;
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