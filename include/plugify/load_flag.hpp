#pragma once

#include <cstdint>

#include "plg/bitmask.hpp"

namespace plugify {
	/**
	 * @enum LoadFlag
	 * @brief Enum class representing various flags for dynamic library loading.
	 *
	 * The values combine as a bitmask. Not every flag applies on every platform;
	 * the ones that do not are ignored, and IPlatformOps reports what the current
	 * platform supports.
	 */
	enum class LoadFlag : uint32_t {
		None = 0,  ///< No flags.

		LazyBinding = 1 << 0,    ///< Delay symbol resolution.
		GlobalSymbols = 1 << 1,  ///< Make symbols globally available.
		NoUnload = 1 << 2,       ///< Prevent unloading.
		DeepBind = 1 << 3,       ///< Prefer local symbols (Linux).
		SecureSearch = 1 << 4,   ///< Use secure search paths (Windows).

		Default = LazyBinding  ///< What the assembly loader uses unless told otherwise.
	};

	/**
	 * @brief Opts @ref LoadFlag into the bitwise operators of plg::bitmask.
	 */
	consteval void enable_bitmask_operators(LoadFlag);
}
