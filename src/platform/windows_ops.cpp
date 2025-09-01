#if PLUGIFY_PLATFORM_WINDOWS

#include "plugify/core/platform_ops.hpp"

#include <windows.h>
#undef LoadLibrary

namespace plugify {
    class WindowsPlatformOps : public IPlatformOps {
    private:
        plg::flat_map<std::filesystem::path, DLL_DIRECTORY_COOKIE> _searchCookies;

        static int TranslateFlags(LoadFlag flags) {
            int winFlags = 0;
            if (flags & LoadFlag::DataOnly)
                winFlags |= LOAD_LIBRARY_AS_DATAFILE;
            if (flags & LoadFlag::SystemOnly)
                winFlags |= LOAD_LIBRARY_SEARCH_SYSTEM32;
            return winFlags;
        }

        static std::string GetLastErrorString() {
            DWORD error = ::GetLastError();
            if (error == 0) return "Unknown error";

            LPSTR buffer = nullptr;
            size_t size = ::FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPSTR>(&buffer), 0, nullptr
            );

            if (size == 0) return std::format("Error code: {}", error);

            std::string message(buffer, size);
            ::LocalFree(buffer);
            return message;
        }

    public:
        ~WindowsPlatformOps() override {
            for (const auto& [path, cookie] : _searchCookies) {
                ::RemoveDllDirectory(cookie);
            }
        }

        Result<void*> LoadLibrary(const std::filesystem::path& path, LoadFlag flags) override {
            HMODULE handle = ::LoadLibraryExW(path.c_str(), nullptr, TranslateFlags(flags));
            if (!handle) {
                return MakeError("Failed to load library '{}': {}",
                                 path.string(), GetLastErrorString());
            }

            if (flags & LoadFlag::NoUnload) {
                HMODULE pinned = nullptr;
                ::GetModuleHandleExW(
                    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                    reinterpret_cast<LPCWSTR>(handle), &pinned
                );
            }

            return handle;
        }

        Result<void> UnloadLibrary(void* handle) override {
            if (!::FreeLibrary(static_cast<HMODULE>(handle))) {
                return MakeError("Failed to unload library: {}", GetLastErrorString());
            }
            return {};
        }

        Result<MemAddr> GetSymbol(void* handle, std::string_view name) override {
            FARPROC proc = ::GetProcAddress(static_cast<HMODULE>(handle), name.data());
            if (!proc) {
                return MakeError("Symbol '{}' not found: {}", name, GetLastErrorString());
            }
            return proc;
        }

        Result<std::filesystem::path> GetLibraryPath(void* handle) override {
            std::wstring path(MAX_PATH, L'\0');
            DWORD size = ::GetModuleFileNameW(static_cast<HMODULE>(handle),
                                              path.data(), static_cast<DWORD>(path.size()));

            if (size == 0) {
                return MakeError("Failed to get module path: {}", GetLastErrorString());
            }

            path.resize(size);
            return std::filesystem::path(path);
        }

        bool SupportsRuntimePathModification() const override { return true; }
        bool SupportsLazyBinding() const override { return false; }

        Result<void> AddSearchPath(const std::filesystem::path& path) override {
            DLL_DIRECTORY_COOKIE cookie = ::AddDllDirectory(path.c_str());
            if (!cookie) {
                return MakeError("Failed to add search path '{}': {}",
                                 path.string(), GetLastErrorString());
            }
            _searchCookies[path] = cookie;
            return {};
        }

        virtual Result<void> RemoveSearchPath(const std::filesystem::path& path) override {
            auto it = _searchCookies.find(path);
            if (it != _searchCookies.end()) {
                if (!::RemoveDllDirectory(it->second)) {
                    return MakeError("Failed to remove search path '{}': {}",
                                     path.string(), GetLastErrorString());
                }
                return {};
            }
            return MakeError("Failed to find search path '{}'", path.string());
        }
    };

    std::unique_ptr<IPlatformOps> CreatePlatformOps() {
        return std::make_unique<WindowsPlatformOps>();
    }
}
#endif
