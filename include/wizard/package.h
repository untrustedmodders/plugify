#pragma once

#include <wizard/version.h>
#include <wizard/descriptor.h>
#include <wizard/package_version.h>
#include <filesystem>
#include <functional>
#include <optional>
#include <memory>
#include <set>

namespace wizard {
	struct PackageVersion {
		std::int32_t version;
		std::uint32_t sdkVersion;
		//std::string checksum;
		//std::vector<std::string> platforms;
		std::vector<std::string> mirrors;

		bool operator <(const PackageVersion& rhs) const { return version > rhs.version; }
	};

	using PackageRef = std::optional<std::reference_wrapper<const PackageVersion>>;

	struct RemotePackage {
		std::string name;
		std::string type;
		std::string author;
		std::string description;
		std::set<PackageVersion> versions;

		bool operator==(const RemotePackage& rhs) const { return name == rhs.name && type == rhs.type; }

		PackageRef LatestVersion() const {
			if (!versions.empty())
				return *versions.begin();
			return {};
		}

		PackageRef Version(std::int32_t version) const {
			auto it = versions.find(PackageVersion{ version, WIZARD_API_VERSION_1_0, {} }); // dummy key for lookup
			if (it != versions.end())
				return *it;
			return {};
		}
	};

	struct LocalPackage {
		std::string name;
		std::string type;
		std::filesystem::path path;
		std::int32_t version;
		std::unique_ptr<Descriptor> descriptor;

		explicit operator RemotePackage() const {
			return { name, type, descriptor->createdBy, descriptor->description, { PackageVersion{ descriptor->version, WIZARD_API_VERSION_1_0, { descriptor->downloadURL }} }};
		}
	};
}