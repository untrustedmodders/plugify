#include "library.h"
#include "os.h"

using namespace wizard;

thread_local static std::string lastError;

Library::Library(void* handle) : _handle{handle} {
}

std::unique_ptr<Library> Library::LoadFromPath(const fs::path& libraryPath) {
#if WIZARD_PLATFORM_WINDOWS || WIZARD_PLATFORM_LINUX
    #if WIZARD_PLATFORM_WINDOWS
        void* handle = static_cast<void*>(LoadLibraryW(libraryPath.c_str()));
#elif WIZARD_PLATFORM_LINUX
        void* handle = dlopen(assemblyPath.string().c_str(), RTLD_LAZY);
#endif // WIZARD_PLATFORM_WINDOWS
        if (handle) {
            return std::unique_ptr<Library>(new Library{handle});
        }
#if WIZARD_PLATFORM_WINDOWS
        uint32_t errorCode = GetLastError();
        if (errorCode != 0) {
            LPSTR messageBuffer = nullptr;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            lastError = std::string(messageBuffer, size);
            LocalFree(messageBuffer);
        }
#elif WIZARD_PLATFORM_LINUX
        lastError = dlerror();
#endif // WIZARD_PLATFORM_WINDOWS
        return nullptr;
#else // !(WIZARD_PLATFORM_WINDOWS || WIZARD_PLATFORM_LINUX)
    return nullptr;
#endif // WIZARD_PLATFORM_WINDOWS || WIZARD_PLATFORM_LINUX
}

Library::~Library() {
#if WIZARD_PLATFORM_WINDOWS
    FreeLibrary(static_cast<HMODULE>(_handle));
#else
    dlclose(_handle);
#endif
}

std::string Library::GetError() {
    return lastError;
}

void* Library::GetFunction(const char* functionName) const {
    if (!_handle) {
        WZ_LOG_ERROR("DLL not loaded.");
        return nullptr;
    }

#if WIZARD_PLATFORM_WINDOWS
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(_handle), functionName));
#else
    return dlsym(_handle, functionName.data());
#endif
}
