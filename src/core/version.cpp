#include <plugify/version.h>

using namespace plugify;

Version::Version(uint8_t major, uint8_t minor, uint8_t patch, uint8_t tweak) : _major{major}, _minor{minor}, _patch{patch}, _tweak{tweak} {
}

Version::Version(uint32_t version) {
	*this = version;
}

Version::operator uint32_t() const {
	return PLUGIFY_MAKE_VERSION(_major, _minor, _patch, _tweak);
}

auto Version::operator<=>(const Version& rhs) const {
	return PLUGIFY_MAKE_VERSION(_major, _minor, _patch, _tweak) <=> PLUGIFY_MAKE_VERSION(rhs._major, rhs._minor, rhs._patch, rhs._tweak);
}

Version& Version::operator=(uint32_t version) {
	_major = PLUGIFY_MAKE_VERSION_MAJOR(version);
	_minor = PLUGIFY_MAKE_VERSION_MINOR(version);
	_patch = PLUGIFY_MAKE_VERSION_PATCH(version);
	_tweak = PLUGIFY_MAKE_VERSION_TWEAK(version);
	return *this;
}

std::string Version::ToString() const {
	return std::format("{}.{}.{}.{}", _major, _minor, _patch, _tweak);
}
