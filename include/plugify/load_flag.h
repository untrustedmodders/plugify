#pragma once

#include <type_traits>

namespace plugify {
	/**
	 * @enum ProtFlag
	 * @brief Enum class representing various flags for dynamic library loading.
	 */
	enum class LoadFlag {
		Default = 0, ///< Value means this gives no information about loading flag

	 	// Unix specific.
		LoadLazy = 1 << 0, ///< Perform lazy binding. Only resolve symbols as they are needed.
		LoadNow = 1 << 1, ///< Perform immediate binding. All symbols are resolved when the library is loaded.
		LoadGlobal = 1 << 2, ///< Make symbols from the library globally available for symbol resolution of subsequently loaded libraries.
		LoadLocal = 1 << 3, ///< Symbols are not made available to resolve references in subsequently loaded libraries.
		LoadNodelete = 1 << 4, ///< Do not unload the library during dlclose.
		LoadNoload = 1 << 5, ///< Do not load the library. Used to test if the library is already loaded.
		LoadDeepbind = 1 << 6, ///< Use the library’s symbols in preference to global symbols with the same name. Linux-specific.

		// Windows specific.
		DontResolveDllReferences = 1 << 7, ///< Do not resolve DLL references.
		LoadAlteredSearchPath = 1 << 8, ///< Use an altered search strategy to locate the DLL.
		LoadAsDatafile = 1 << 9, ///< Load the DLL as a data file. Does not execute its code or run any initialization routines.
		LoadAsDatafileExclusive = 1 << 10, ///< Load the DLL as a data file exclusively.
		LoadAsImageResource = 1 << 11, ///< Load the DLL as an image resource.
		LoadSearchApplicationDir = 1 << 12, ///< Search the application's directory first.
		LoadSearchDefaultDirs = 1 << 13, ///< Use default search directories to locate the DLL.
		LoadSearchDllLoadDir = 1 << 14, ///< Search the directory that contains the DLL.
		LoadSearchSystem32 = 1 << 15, ///< Search the System32 directory.
		LoadSearchUserDirs = 1 << 16, ///< Search directories added using the AddDllDirectory function.
		LoadRequireSignedTarget = 1 << 17, ///< Require that the loaded target is signed.
		LoadSafeCurrentDirs = 1 << 18, ///< Use safe current directories for library search.
		LoadIgnoreAuthzLevel = 1 << 19, ///< Ignore the authorization level of the calling process.
		PinInMemory = 1 << 20 ///< Do not unload the library during FreeLibrary.
	};

	/**
	 * @brief Bitwise OR operator for LoadFlag enum class.
	 * @param lhs Left-hand side LoadFlag.
	 * @param rhs Right-hand side LoadFlag.
	 * @return Result of the bitwise OR operation.
	 */
	inline LoadFlag operator|(LoadFlag lhs, LoadFlag rhs) {
		using underlying = typename std::underlying_type<LoadFlag>::type;
		return static_cast<LoadFlag>(
				static_cast<underlying>(lhs) | static_cast<underlying>(rhs)
		);
	}

	/**
	 * @brief Bitwise AND operator for LoadFlag enum class.
	 * @param lhs Left-hand side LoadFlag.
	 * @param rhs Right-hand side LoadFlag.
	 * @return Result of the bitwise AND operation.
	 */
	inline bool operator&(LoadFlag lhs, LoadFlag rhs) {
		using underlying = typename std::underlying_type<LoadFlag>::type;
		return static_cast<underlying>(lhs) & static_cast<underlying>(rhs);
	}

	/**
	 * @brief Bitwise OR assignment operator for LoadFlag enum class.
	 * @param lhs Left-hand side LoadFlag.
	 * @param rhs Right-hand side LoadFlag.
	 * @return Reference to the left-hand side LoadFlag.
	 */
	inline LoadFlag& operator|=(LoadFlag& lhs, LoadFlag rhs) {
		lhs = lhs | rhs;
		return lhs;
	}
}