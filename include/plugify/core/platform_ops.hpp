#pragma once

#include <memory>

#include "plugify/core/types.hpp"
#include "plugify/core/mem_addr.hpp"
#include "plugify/core/load_flag.hpp"

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
        virtual Result<void> AddSearchPath(const std::filesystem::path& path) {
            return MakeError("Runtime path modification not supported on this platform");
        }

        virtual Result<void> RemoveSearchPath(const std::filesystem::path& path) {
            return MakeError("Runtime path modification not supported on this platform");
        }
    };

    // Factory function (implemented per platform)
    std::unique_ptr<IPlatformOps> CreatePlatformOps();
}
