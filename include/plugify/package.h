#pragma once

#include <plugify/descriptor.h>
#include <filesystem>
#include <functional>
#include <optional>
#include <memory>
#include <set>

namespace plugify {
	struct PackageVersion {
		std::int32_t version;
		std::string checksum;
		std::string download;
		std::vector<std::string> platforms;
		// TODO: dependencies, conflicts here may be?

		bool operator <(const PackageVersion& rhs) const { return version > rhs.version; }
	};

	using PackageOpt = std::optional<std::reference_wrapper<const PackageVersion>>;

	struct Package {
		std::string name;
		std::string type;

		bool operator==(const Package& rhs) const { return name == rhs.name && type == rhs.type; }
	};

	struct RemotePackage : public Package {
		std::string author;
		std::string description;
		std::set<PackageVersion> versions;

		PackageOpt LatestVersion() const {
			if (!versions.empty())
				return *versions.begin();
			return {};
		}

		PackageOpt Version(std::int32_t version) const {
			auto it = versions.find(PackageVersion{ version, {}, {}, {} }); // dummy key for lookup
			if (it != versions.end())
				return *it;
			return {};
		}
	};

	struct LocalPackage : public Package {
		std::filesystem::path path;
		std::int32_t version;
		std::shared_ptr<Descriptor> descriptor;

		explicit operator RemotePackage() const {
			return { name, type, descriptor->createdBy, descriptor->description, { PackageVersion{ descriptor->version, {}, descriptor->downloadURL, descriptor->supportedPlatforms} }};
		}
	};
}