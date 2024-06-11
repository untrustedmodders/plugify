#include "library.h"
#include "os.h"

using namespace plugify;

thread_local static std::string lastError;

std::unique_ptr<Library> Library::LoadFromPath(fs::path libraryPath) {
#if PLUGIFY_PLATFORM_WINDOWS
	void* handle = static_cast<void*>(LoadLibraryExW(libraryPath.c_str(), NULL,
		LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR));
#elif PLUGIFY_PLATFORM_LINUX || PLUGIFY_PLATFORM_APPLE
	void* handle = dlopen(libraryPath.string().c_str(), RTLD_LAZY | RTLD_GLOBAL);
#else
	void* handle = nullptr;
#endif
	if (handle) {
		return std::unique_ptr<Library>(new Library(handle));
	}

#if PLUGIFY_PLATFORM_WINDOWS
	DWORD errorCode = GetLastError();
	if (errorCode != 0) {
		LPSTR messageBuffer = NULL;
		DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		lastError = std::string(messageBuffer, size);
		LocalFree(messageBuffer);
	}
#elif PLUGIFY_PLATFORM_LINUX || PLUGIFY_PLATFORM_APPLE
	lastError = dlerror();
#endif
	return nullptr;
}

Library::Library(void* handle) : _handle{handle} {
}

Library::Library(Library&& rhs) noexcept : _handle{rhs._handle} {
	rhs._handle = nullptr;
}

Library::~Library() {
#if PLUGIFY_PLATFORM_WINDOWS
	FreeLibrary(static_cast<HMODULE>(_handle));
#elif PLUGIFY_PLATFORM_LINUX || PLUGIFY_PLATFORM_APPLE
	dlclose(_handle);
#endif
}

Library& Library::operator=(Library&& rhs) noexcept {
	if (this != &rhs) {
#if PLUGIFY_PLATFORM_WINDOWS
		FreeLibrary(static_cast<HMODULE>(_handle));
#elif PLUGIFY_PLATFORM_LINUX || PLUGIFY_PLATFORM_APPLE
		dlclose(_handle);
#endif
		_handle = nullptr;
		std::swap(_handle, rhs._handle);
	}
	return *this;
}

std::string Library::GetError() {
	return lastError;
}

void* Library::GetFunction(std::string_view functionName) const {
	if (!_handle) {
		PL_LOG_ERROR("DLL not loaded.");
		return nullptr;
	}

#if PLUGIFY_PLATFORM_WINDOWS
	return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(_handle), functionName.data()));
#elif PLUGIFY_PLATFORM_LINUX || PLUGIFY_PLATFORM_APPLE
	return dlsym(_handle, functionName.data());
#else
	return nullptr;
#endif
}
