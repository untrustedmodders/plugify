#pragma once

#include "plugify/asm/mem_addr.hpp"
#include "plugify/asm/prot_flag.hpp"

namespace plugify {
	/**
	 * @class MemAccessor
	 * @brief A class providing various memory access routines.
	 */
	class MemAccessor {
	public:
		/**
		 * @brief Defines a memory read/write routine that may fail ungracefully.
		 * It's expected this library will only ever use this routine in cases that are expected to succeed.
		 * @param dest The destination memory address.
		 * @param src The source memory address.
		 * @param size The number of bytes to copy.
		 * @return True if the memory copy succeeds, false otherwise.
		 */
		static bool MemCopy(MemAddr dest, MemAddr src, size_t size);

		/**
		 * @brief Defines a memory write routine that will not throw exceptions, and can handle potential
		 * writes to NO_ACCESS or otherwise inaccessible memory pages. Defaults to WriteProcessMemory.
		 * Must fail gracefully.
		 * @param dest The destination memory address.
		 * @param src The source memory address.
		 * @param size The number of bytes to copy.
		 * @param written A reference to the variable that will hold the number of bytes successfully written.
		 * @return True if the memory copy succeeds, false otherwise.
		 */
		static bool SafeMemCopy(MemAddr dest, MemAddr src, size_t size, size_t& written) noexcept;

		/**
		 * @brief Defines a memory read routine that will not throw exceptions, and can handle potential
		 * reads from NO_ACCESS or otherwise inaccessible memory pages. Defaults to ReadProcessMemory.
		 * Must fail gracefully.
		 * @param src The source memory address.
		 * @param dest The destination memory address.
		 * @param size The number of bytes to read.
		 * @param read A reference to the variable that will hold the number of bytes successfully read.
		 * @return True if the memory read succeeds, false otherwise.
		 */
		static bool SafeMemRead(MemAddr src, MemAddr dest, size_t size, size_t& read) noexcept;

		/**
		 * @brief Defines a memory protection set/unset routine that may fail ungracefully.
		 * @param dest The memory address to change protection for.
		 * @param size The number of bytes to change protection for.
		 * @param newProtection The new protection flags to set.
		 * @param status A reference to a variable that will hold the success status of the operation.
		 * @return The previous protection flags if the operation succeeds, otherwise an appropriate error code.
		 */
		static ProtFlag MemProtect(MemAddr dest, size_t size, ProtFlag newProtection, bool& status);
	};
} // namespace plugify
