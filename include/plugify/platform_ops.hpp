#pragma once

#include <memory>
#include <filesystem>

#include "plugify/load_flag.hpp"
#include "plugify/address.hpp"
#include "plugify/types.hpp"

namespace plugify {
	/**
	 * @class IPlatformOps
	 * @brief Interface over the dynamic-loading API of the host platform.
	 *
	 * One implementation exists per platform, wrapping the native loader
	 * (`LoadLibrary` on Windows, `dlopen` elsewhere) behind a uniform,
	 * error-returning interface. Not every platform supports every operation, so
	 * the capability queries say what is available before it is used, and the
	 * search-path methods have defaults that report the operation as unsupported.
	 *
	 * @see CreatePlatformOps, IAssemblyLoader
	 */
	class IPlatformOps {
	public:
		/**
		 * @brief Virtual destructor.
		 */
		virtual ~IPlatformOps() = default;

		// Core operations

		/**
		 * @brief Load a shared library.
		 * @param path Path to the library file.
		 * @param flags Loading flags.
		 * @return The native handle of the library, or an error message.
		 */
		virtual Result<void*> LoadLibrary(const std::filesystem::path& path, LoadFlag flags) = 0;

		/**
		 * @brief Unload a shared library.
		 * @param handle The native handle of the library.
		 * @return Nothing on success, an error message otherwise.
		 */
		virtual Result<void> UnloadLibrary(void* handle) = 0;

		/**
		 * @brief Look a symbol up in a loaded library.
		 * @param handle The native handle of the library.
		 * @param name Name of the symbol.
		 * @return Address of the symbol, or an error message if it was not found.
		 */
		virtual Result<Address> GetSymbol(void* handle, std::string_view name) = 0;

		/**
		 * @brief Ask the loader where a library was loaded from.
		 * @param handle The native handle of the library.
		 * @return Path of the library file, or an error message.
		 */
		virtual Result<std::filesystem::path> GetLibraryPath(void* handle) = 0;

		// Platform capabilities

		/**
		 * @brief Check whether the search path can be changed at runtime.
		 * @return True if @ref AddSearchPath and @ref RemoveSearchPath do anything on this platform.
		 */
		[[nodiscard]] virtual bool SupportsRuntimePathModification() const = 0;

		/**
		 * @brief Check whether symbol resolution can be deferred.
		 * @return True if @ref LoadFlag::LazyBinding has an effect on this platform.
		 */
		[[nodiscard]] virtual bool SupportsLazyBinding() const = 0;

		// Search path management (if supported)

		/**
		 * @brief Add a directory to the library search path.
		 * @param path The directory to add.
		 * @return Nothing on success, an error message on a platform that cannot do this.
		 */
		virtual Result<void> AddSearchPath([[maybe_unused]] const std::filesystem::path& path) {
			return MakeError("Runtime path modification not supported on this platform!");
		}

		/**
		 * @brief Remove a directory from the library search path.
		 * @param path The directory to remove.
		 * @return Nothing on success, an error message on a platform that cannot do this.
		 */
		virtual Result<void> RemoveSearchPath([[maybe_unused]] const std::filesystem::path& path) {
			return MakeError("Runtime path modification not supported on this platform?");
		}
	};

	/**
	 * @brief Create the platform operations for the platform this build targets.
	 * @return The implementation for the current platform.
	 */
	std::shared_ptr<IPlatformOps> CreatePlatformOps();
}
