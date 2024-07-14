#pragma once

#include <plugify/mem_addr.h>
#include <plugify/prot_flag.h>
#include <plugify_export.h>

namespace plugify {
	class MemAccessor;
	/**
	 * @class MemProtector
	 * @brief A class to manage memory protection settings.
	 *
	 * This class allows setting and unsetting memory protection for a given
	 * memory region. It ensures that the original protection settings are restored
	 * when the object is destroyed.
	 */
	class PLUGIFY_API MemProtector {
	public:
		MemProtector() = delete; /**< Deleted default constructor. */

		/**
		 * @brief Constructs a MemProtector object to set memory protection.
		 *
		 * @param address The memory address to protect.
		 * @param length The length of the memory region to protect.
		 * @param prot The protection flags to set.
		 * @param unsetOnDestroy If true, the original protection settings will be restored when the object is destroyed.
		 */
		MemProtector(MemAddr address, size_t length, ProtFlag prot, bool unsetOnDestroy = true);

		/**
		 * @brief Destructor to restore the original protection settings if required.
		 */
		~MemProtector();

		/**
		 * @brief Gets the original protection settings.
		 *
		 * @return The original protection flags.
		 */
		ProtFlag OriginalProt() const noexcept { return _origProtection; }

		/**
		 * @brief Checks if the memory protection was successfully set.
		 *
		 * @return True if the memory protection was successfully set, false otherwise.
		 */
		bool IsValid() const noexcept { return _status; }

	private:
		void* _address; /**< The memory address to protect. */
		size_t _length; /**< The length of the memory region to protect. */
		bool _status; /**< The status of the memory protection operation. */
		bool _unsetLater; /**< Whether to unset the protection on destruction. */

		ProtFlag _origProtection{ ProtFlag::UNSET }; /**< The original protection flags. */
	};

	/**
	 * @brief Translates protection flags to an integer representation.
	 *
	 * @param flags The protection flags to translate.
	 * @return An integer representation of the protection flags.
	 */
	int TranslateProtection(ProtFlag flags) noexcept;

	/**
	 * @brief Translates an integer representation of protection flags to ProtFlag.
	 *
	 * @param prot The integer representation of the protection flags.
	 * @return The corresponding ProtFlag.
	 */
	ProtFlag TranslateProtection(int prot) noexcept;

} // namespace plugify