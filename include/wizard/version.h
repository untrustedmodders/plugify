#pragma once

#include <cstdint>
#include <wizard_export.h>

namespace wizard {
	class WIZARD_API Version {
	public:
		Version(uint8_t major, uint8_t minor, uint8_t patch, uint8_t tweak);
		explicit Version(uint32_t version);
		Version& operator=(uint32_t version);

		operator uint32_t() const;
		auto operator<=>(const Version& rhs) const;

		std::string ToString() const;

	private:
		uint8_t _major;
		uint8_t _minor;
		uint8_t _patch;
		uint8_t _tweak;
	};
}

#define WIZARD_MAKE_VERSION(major, minor, patch, tweak) ((((uint32_t)(major)) << 29) | (((uint32_t)(minor)) << 22) | (((uint32_t)(patch)) << 12) | ((uint32_t)(tweak)))
#define WIZARD_MAKE_VERSION_MAJOR(version) uint8_t((uint32_t)(version) >> 29)
#define WIZARD_MAKE_VERSION_MINOR(version) uint8_t(((uint32_t)(version) >> 22) & 0x7FU)
#define WIZARD_MAKE_VERSION_PATCH(version) uint8_t(((uint32_t)(version) >> 12) & 0x3FFU)
#define WIZARD_MAKE_VERSION_TWEAK(version) uint8_t((uint32_t)(version) & 0xFFFU)

#define WIZARD_API_VERSION_1_0 WIZARD_MAKE_VERSION(1, 0, 0, 0)
