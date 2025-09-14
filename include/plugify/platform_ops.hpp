#pragma once

#include <memory>

#include "plugify/load_flag.hpp"
#include "plugify/mem_addr.hpp"
#include "plugify/types.hpp"

namespace plugify {
	// Platform-specific operations interface
	class IPlatformOps {
	public:
		virtual ~IPlatformOps() = default;

		// Core operations
		virtual Result<void*> LoadLibrary(const std::filesystem::path& path, LoadFlag flags) = 0;
		virtual Result<void> UnloadLibrary(void* handle) = 0;
		virtual Result<MemAddr> GetSymbol(void* handle, std::string_view name) = 0;
		virtual Result<std::filesystem::path> GetLibraryPath(void* handle) = 0;

		// Platform capabilities
		[[nodiscard]] virtual bool SupportsRuntimePathModification() const = 0;
		[[nodiscard]] virtual bool SupportsLazyBinding() const = 0;

		// Search path management (if supported)
		virtual Result<void> AddSearchPath([[maybe_unused]] const std::filesystem::path& path) {
			return MakeError("Runtime path modification not supported on this platform");
		}

		virtual Result<void> RemoveSearchPath([[maybe_unused]] const std::filesystem::path& path) {
			return MakeError("Runtime path modification not supported on this platform");
		}
	};

	// Factory function (implemented per platform)
	std::shared_ptr<IPlatformOps> CreatePlatformOps();
}
