#include "library.h"

#ifdef _WIN32
#include <windows.h>
#undef ERROR
#else
#include <dlfcn.h>
#endif

using namespace wizard;

Library::Library(const fs::path& libraryPath) {
#ifdef _WIN32
    _handle = static_cast<void*>(LoadLibraryA(libraryPath.string().c_str()));
    if (!_handle) {
        WIZARD_LOG("Failed to load DLL.", ErrorLevel::ERROR);
    }
#else
    _handle = dlopen(libraryPath, RTLD_LAZY);
    if (!_handle) {
        WIZARD_LOG(Failed to load shared library: + dlerror(), ErrorLevel::ERROR);
    }
#endif
}

Library::~Library() {
    if (_handle) {
#ifdef _WIN32
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

#ifdef _WIN32
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(_handle), functionName.data()));
#else
    return dlsym(_handle, functionName.data());
#endif
}
