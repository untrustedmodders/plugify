#pragma once

#include <cstdint>
#include <plugify_export.h>

namespace plugify {
	/**
	 * @brief Represents a version number with major, minor, patch, and tweak components.
	 */
	class PLUGIFY_API Version final {
	public:
		/**
		 * @brief Constructor to initialize a Version object with individual components.
		 * @param major The major version component.
		 * @param minor The minor version component.
		 * @param patch The patch version component.
		 * @param tweak The tweak version component.
		 */
		Version(uint8_t major, uint8_t minor, uint8_t patch, uint8_t tweak) noexcept;

		/**
		 * @brief Explicit constructor to initialize a Version object from a packed 32-bit version number.
		 * @param version The packed 32-bit version number.
		 */
		explicit Version(uint32_t version) noexcept;

		/**
		 * @brief Assignment operator to set the Version object from a packed 32-bit version number.
		 * @param version The packed 32-bit version number.
		 * @return Reference to the modified Version object.
		 */
		Version& operator=(uint32_t version) noexcept;

		/**
		 * @brief Conversion operator to retrieve the packed 32-bit version number.
		 * @return The packed 32-bit version number.
		 */
		operator uint32_t() const noexcept;

		/**
		 * @brief Comparison operator (<=>) for comparing two Version objects.
		 * @param rhs The right-hand side Version object for comparison.
		 * @return The result of the comparison.
		 */
		auto operator<=>(const Version& rhs) const noexcept;

		/**
		 * @brief Get a string representation of the Version object.
		 * @return A string representing the Version object.
		 */
		std::string ToString() const;

	private:
		uint8_t _major; ///< The major version component.
		uint8_t _minor; ///< The minor version component.
		uint8_t _patch; ///< The patch version component.
		uint8_t _tweak; ///< The tweak version component.
	};
} // namespace plugify

#define PLUGIFY_MAKE_VERSION(major, minor, patch, tweak) ((((uint32_t)(major)) << 29) | (((uint32_t)(minor)) << 22) | (((uint32_t)(patch)) << 12) | ((uint32_t)(tweak)))
#define PLUGIFY_MAKE_VERSION_MAJOR(version) uint8_t((uint32_t)(version) >> 29)
#define PLUGIFY_MAKE_VERSION_MINOR(version) uint8_t(((uint32_t)(version) >> 22) & 0x7FU)
#define PLUGIFY_MAKE_VERSION_PATCH(version) uint8_t(((uint32_t)(version) >> 12) & 0x3FFU)
#define PLUGIFY_MAKE_VERSION_TWEAK(version) uint8_t((uint32_t)(version) & 0xFFFU)

#define PLUGIFY_API_VERSION_1_0 PLUGIFY_MAKE_VERSION(1, 0, 0, 0)
