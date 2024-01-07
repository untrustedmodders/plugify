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
		case PackageError::InvalidURL:
			return "Invalid URL";
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
