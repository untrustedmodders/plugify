#include "package.h"

using namespace wizard;

std::string PackageState::GetProgress(int barWidth) const {
	std::stringstream ss;
	ss << "[";
	int pos = static_cast<int>(static_cast<float>(barWidth) * (ratio / 100.0f));
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) ss << "=";
		else if (i == pos) ss << ">";
		else ss << " ";
	}
	ss << "] " << static_cast<int>(ratio) << " %\n";
	return ss.str();
}

std::string_view PackageState::GetError() const {
	switch (error) {
		case PackageError::FailedReadingArchive:
			return "Failed reading archive";
		case PackageError::FailedCreatingDirectory:
			return "Failed creating directory";
		case PackageError::FailedWritingToDisk:
			return "Failed writing to disk";
		case PackageError::PackageFetchingFailed:
			return "Package fetching failed";
		case PackageError::PackageAuthorizationFailed:
			return "Package authorization failed";
		case PackageError::PackageCorrupted:
			return "Downloaded archive checksum does not match verified hash";
		case PackageError::NoMemoryAvailable:
			return "No memory available";
		case PackageError::NotFound:
			return "Package not found, not currently being downloaded";
		case PackageError::None:
		default:
			return "";
	}
}

LocalPackage::operator RemotePackage() const {
	return { name, type, descriptor->createdBy, descriptor->description, { PackageVersion{ descriptor->version, WIZARD_API_VERSION_1_0, { descriptor->downloadURL }} }};
}

bool PackageVersion::operator<(const PackageVersion& rhs) const {
	return version > rhs.version;
}

bool RemotePackage::operator==(const RemotePackage& rhs) const {
	return name == rhs.name && type == rhs.type;
}

PackageRef RemotePackage::LatestVersion() const {
	if (!versions.empty())
		return *versions.begin();
	return {};
}

PackageRef RemotePackage::Version(const int32_t version) const{
	PackageVersion key{ version, WIZARD_API_VERSION_1_0, {} };
	auto it = versions.find(key);
	if (it != versions.end())
		return *it;
	return {};
}
