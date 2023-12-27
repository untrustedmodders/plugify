#include "library.h"
#include "os.h"

using namespace wizard;

Library::Library(const fs::path& libraryPath) {
#if WIZARD_PLATFORM_WINDOWS
    _handle = static_cast<void*>(LoadLibraryA(libraryPath.string().c_str()));
    if (!_handle) {
        WIZARD_LOG("Failed to load DLL.", ErrorLevel::ERROR);
    }
#else
    _handle = dlopen(libraryPath.string().c_str(), RTLD_LAZY);
    if (!_handle) {
        WIZARD_LOG("Failed to load shared library: " + std::string(dlerror()), ErrorLevel::ERROR);
    }
#endif
}

Library::~Library() {
    if (_handle) {
#if WIZARD_PLATFORM_WINDOWS
        FreeLibrary(static_cast<HMODULE>(_handle));
#else
        dlclose(_handle);
#endif
    }
}

void* Library::GetFunction(std::string_view functionName) {
    if (!_handle) {
        WIZARD_LOG("DLL not loaded.", ErrorLevel::ERROR);
        return nullptr;
    }

#if WIZARD_PLATFORM_WINDOWS
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(_handle), functionName.data()));
#else
    return dlsym(_handle, functionName.data());
#endif
}
