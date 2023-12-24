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
    m_handle = static_cast<void*>(LoadLibraryA(libraryPath.string().c_str()));
    if (!m_handle) {
        WIZARD_LOG("Failed to load DLL.", ErrorLevel::ERROR);
    }
#else
    m_handle = dlopen(libraryPath, RTLD_LAZY);
    if (!m_handle) {
        WIZARD_LOG(Failed to load shared library: + dlerror(), ErrorLevel::ERROR);
    }
#endif
}

Library::~Library() {
    if (m_handle) {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(m_handle));
#else
        dlclose(m_handle);
#endif
    }
}

void* Library::GetFunction(std::string_view functionName) {
    if (!m_handle) {
        WIZARD_LOG("DLL not loaded.", ErrorLevel::ERROR);
        return nullptr;
    }

#ifdef _WIN32
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(m_handle), functionName.data()));
#else
    return dlsym(m_handle, functionName.data());
#endif
}
