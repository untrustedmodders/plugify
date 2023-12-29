#include <wizard/version.h>

using namespace wizard;

Version::Version(uint8_t major, uint8_t minor, uint8_t patch, uint8_t tweak) : _major{major}, _minor{minor}, _patch{patch}, _tweak{tweak} {
}

Version::Version(uint32_t version) {
    *this = version;
}

Version::operator uint32_t() const {
    return WIZARD_MAKE_VERSION(_major, _minor, _patch, _tweak);
}

auto Version::operator<=>(const Version& rhs) const {
    return WIZARD_MAKE_VERSION(_major, _minor, _patch, _tweak) <=> WIZARD_MAKE_VERSION(rhs._major, rhs._minor, rhs._patch, rhs._tweak);
}

Version& Version::operator=(uint32_t version) {
    _major = WIZARD_MAKE_VERSION_MAJOR(version);
    _minor = WIZARD_MAKE_VERSION_MINOR(version);
    _patch = WIZARD_MAKE_VERSION_PATCH(version);
    _tweak = WIZARD_MAKE_VERSION_TWEAK(version);
    return *this;
}

std::string Version::ToString() const {
    return WZ_FMT::format("{}.{}.{}.{}", _major, _minor, _patch, _tweak);
}
