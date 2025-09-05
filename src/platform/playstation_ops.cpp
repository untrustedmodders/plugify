#include "plugify/platform_ops.hpp"

#include <kernel.h>

namespace plugify {
    class PlayStationPlatformOps : public IPlatformOps {
    private:
        static std::string TranslateError(int error) {
            switch (error) {
                case SCE_KERNEL_ERROR_EINVAL:
                    return "Invalid arguments";
                case SCE_KERNEL_ERROR_ENOENT:
                    return "File does not exist";
                case SCE_KERNEL_ERROR_ENOEXEC:
                    return "Invalid file format";
                case SCE_KERNEL_ERROR_ENOMEM:
                    return "Insufficient memory";
                case SCE_KERNEL_ERROR_EACCES:
                    return "Access denied";
                case SCE_KERNEL_ERROR_EFAULT:
                    return "Invalid memory address";
                case SCE_KERNEL_ERROR_EAGAIN:
                    return "Insufficient resources";
                case SCE_KERNEL_ERROR_ESDKVERSION:
                    return "SDK version mismatch";
                case SCE_KERNEL_ERROR_ESTART:
                    return "module_start() failed";
                case SCE_KERNEL_ERROR_ENODYNLIBMEM:
                    return "Insufficient system memory";
                default:
                    return std::format("Unknown error: 0x{:08X}", error);
            }
        }

    public:
        Result<void*> LoadLibrary(const std::filesystem::path& path, LoadFlag flags) override {
            // PlayStation doesn't use flags in the same way
            SceKernelModule handle = sceKernelLoadStartModule(
                path.c_str(), 0, nullptr, 0, nullptr, nullptr
            );

            if (handle < 0) {
                return MakeError("Failed to load module '{}': {}",
                                 plg::as_string(path), TranslateError(handle));
            }

            return reinterpret_cast<void*>(handle);
        }

        Result<void> UnloadLibrary(void* handle) override {
            int ret = sceKernelStopUnloadModule(
                reinterpret_cast<SceKernelModule>(handle),
                0, nullptr, 0, nullptr, nullptr
            );

            if (ret < 0) {
                return MakeError("Failed to unload module: {}", TranslateError(ret));
            }

            return {};
        }

        Result<MemAddr> GetSymbol(void* handle, std::string_view name) override {
            void* address = nullptr;
            int ret = sceKernelDlsym(
                reinterpret_cast<SceKernelModule>(handle),
                name.data(),
                &address
            );

            if (ret < 0) {
                return MakeError("Symbol '{}' not found: {}", name, TranslateError(ret));
            }

            return address;
        }

        Result<std::filesystem::path> GetLibraryPath(void* handle) override {
            // PlayStation doesn't provide a direct way to get module path
            // Would need to maintain a mapping or use module info structures
            return MakeError("GetLibraryPath not implemented for PlayStation");
        }

        bool SupportsRuntimePathModification() const override { return false; }
        bool SupportsLazyBinding() const override { return false; }
    };

    std::shared_ptr<IPlatformOps> CreatePlatformOps() {
        return std::make_shared<PlayStationPlatformOps>();
    }
}
