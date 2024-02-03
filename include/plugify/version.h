#pragma once

#include <cstdint>
#include <plugify_export.h>

namespace plugify {
	class PLUGIFY_API Version {
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

#define PLUGIFY_MAKE_VERSION(major, minor, patch, tweak) ((((uint32_t)(major)) << 29) | (((uint32_t)(minor)) << 22) | (((uint32_t)(patch)) << 12) | ((uint32_t)(tweak)))
#define PLUGIFY_MAKE_VERSION_MAJOR(version) uint8_t((uint32_t)(version) >> 29)
#define PLUGIFY_MAKE_VERSION_MINOR(version) uint8_t(((uint32_t)(version) >> 22) & 0x7FU)
#define PLUGIFY_MAKE_VERSION_PATCH(version) uint8_t(((uint32_t)(version) >> 12) & 0x3FFU)
#define PLUGIFY_MAKE_VERSION_TWEAK(version) uint8_t((uint32_t)(version) & 0xFFFU)

#define PLUGIFY_API_VERSION_1_0 PLUGIFY_MAKE_VERSION(1, 0, 0, 0)
