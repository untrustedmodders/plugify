#pragma once

#include <cstdint>

#include "plg/bitmask.hpp"

namespace plugify {
	/**
	 * @enum LoadFlag
	 * @brief Enum class representing various flags for dynamic library loading.
	 */
	enum class LoadFlag : uint32_t {
		None = 0,

		LazyBinding = 1 << 0,    // Delay symbol resolution
		GlobalSymbols = 1 << 1,  // Make symbols globally available
		NoUnload = 1 << 2,       // Prevent unloading
		DeepBind = 1 << 3,       // Prefer local symbols (Linux)
		SecureSearch = 1 << 4,   // Use secure search paths (Windows)

		Default = LazyBinding
	};

	consteval void enable_bitmask_operators(LoadFlag);
}
