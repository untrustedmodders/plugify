#if PLUGIFY_PLATFORM_SWITCH

#include "plugify/core/platform_ops.hpp"

#include <nn/ro.h>
#include <nn/fs.h>
#include <nn/os.h>
#include <cstdlib>

namespace plugify {
    class SwitchPlatformOps : public IPlatformOps {
    private:
        struct NroHandle {
            nn::ro::Module module{};
            std::unique_ptr<uint8_t[]> data;
            int64_t dataSize{};
            std::unique_ptr<uint8_t[]> bssData;
            size_t bssSize{};

            ~NroHandle() {
                if (dataSize && bssSize) {
                    nn::ro::UnloadModule(&module);
                }
            }
        };

        static int TranslateFlags(LoadFlag flags) {
            int nnFlags = 0;
            if (flags & LoadFlag::LazyBinding)
                nnFlags |= nn::ro::BindFlag::BindFlag_Lazy;
            else
                nnFlags |= nn::ro::BindFlag::BindFlag_Now;
            return nnFlags;
        }

        static std::string TranslateError(nn::Result result) {
            if (nn::ro::ResultInvalidNroImage::Includes(result)) {
                return "Invalid NRO image";
            } else if (nn::ro::ResultOutOfAddressSpace::Includes(result)) {
                return "Insufficient address space";
            } else if (nn::ro::ResultNroAlreadyLoaded::Includes(result)) {
                return "NRO already loaded";
            } else if (nn::ro::ResultMaxModule::Includes(result)) {
                return "Maximum module limit reached";
            } else if (nn::ro::ResultNotAuthorized::Includes(result)) {
                return "Authentication failed";
            }
            return std::format("Unknown error: module={} desc={} value=0x{:08X}",
                               result.GetModule(), result.GetDescription(),
                               result.GetInnerValueForDebug());
        }

    public:
        Result<void*> LoadLibrary(const std::filesystem::path& path, LoadFlag flags) override {
            // Open and read the NRO file
            nn::fs::FileHandle file;
            nn::Result ret = nn::fs::OpenFile(&file, path.c_str(), nn::fs::OpenMode_Read);
            if (ret.IsFailure()) {
                return MakeError("Failed to open file '{}': {}",
                                 path.string(), TranslateError(ret));
            }

            // Get file size
            int64_t fileSize;
            ret = nn::fs::GetFileSize(&fileSize, file);
            if (ret.IsFailure()) {
                nn::fs::CloseFile(file);
                return MakeError("Failed to get file size: {}", TranslateError(ret));
            }

            // Allocate aligned memory for NRO data
            auto handle = std::make_unique<NroHandle>();
            handle->dataSize = fileSize;
            handle->data.reset(static_cast<uint8_t*>(
                std::aligned_alloc(nn::os::MemoryPageSize, fileSize)
            ));

            // Read file
            size_t bytesRead;
            ret = nn::fs::ReadFile(&bytesRead, file, 0, handle->data.get(), fileSize);
            nn::fs::CloseFile(file);

            if (ret.IsFailure() || bytesRead != fileSize) {
                return MakeError("Failed to read file: {}", TranslateError(ret));
            }

            // Get BSS size
            ret = nn::ro::GetBufferSize(&handle->bssSize, handle->data.get());
            if (ret.IsFailure()) {
                return MakeError("Failed to get buffer size: {}", TranslateError(ret));
            }

            // Allocate BSS
            handle->bssData.reset(static_cast<uint8_t*>(
                std::aligned_alloc(nn::os::MemoryPageSize, handle->bssSize)
            ));

            // Load the module
            ret = nn::ro::LoadModule(&handle->module, handle->data.get(),
                                     handle->bssData.get(), handle->bssSize, TranslateFlags(flags));

            if (ret.IsFailure()) {
                return MakeError("Failed to load module: {}", TranslateError(ret));
            }

            return handle.release();
        }

        Result<void> UnloadLibrary(void* handle) override {
            // NroHandle destructor handles cleanup
            delete static_cast<NroHandle*>(handle);
            return {};
        }

        Result<MemAddr> GetSymbol(void* handle, std::string_view name) override {
            auto* nroHandle = static_cast<NroHandle*>(handle);
            uintptr_t address = 0;

            nn::Result ret = nn::ro::LookupModuleSymbol(
                &address, &nroHandle->module, name.data()
            );

            if (ret.IsFailure()) {
                return MakeError("Symbol '{}' not found", name);
            }

            return address;
        }

        Result<std::filesystem::path> GetLibraryPath(void* handle) override {
            // Switch doesn't provide a way to get module path
            return MakeError("GetLibraryPath not implemented for Switch");
        }

        bool SupportsRuntimePathModification() const override { return false; }
        bool SupportsLazyBinding() const override { return true; }
    };

    std::unique_ptr<IPlatformOps> CreatePlatformOps() {
        return std::make_unique<SwitchPlatformOps>();
    }
}
#endif
