#if PLUGIFY_PLATFORM_ORBIS || PLUGIFY_PLATFORM_PROSPERO

#include <plugify/assembly.h>

#include "os.h"

using namespace plugify;

Assembly::~Assembly() {
	if (_handle) {
		[[maybe_unused]] int ret = sceKernelStopUnloadModule(reinterpret_cast<SceKernelModule>(_handle), 0, nullptr, 0, nullptr, nullptr);
#if PLUGIFY_LOGGING
		switch (ret) {
			case SCE_KERNEL_ERROR_EINVAL:
				PL_LOG_VERBOSE("Assembly::~Assembly() - '{}': flags or pOpt is invalid", _path.c_str());
				break;
			case SCE_KERNEL_ERROR_ESRCH:
				PL_LOG_VERBOSE("Assembly::~Assembly() - '{}': handle is invalid (specified dynamic library is not loaded)", _path.c_str());
				break;
			case SCE_KERNEL_ERROR_EBUSY:
				PL_LOG_VERBOSE("Assembly::~Assembly() - '{}': Specified dynamic library is referenced by a thread other than the thread that called this function", _path.c_str());
				break;
			default:
				break;
		}
#endif // PLUGIFY_LOGGING
		_handle = nullptr;
	}
}

bool Assembly::InitFromName(std::string_view /*moduleName*/, LoadFlag /*flags*/, const SearchDirs& /*additionalSearchDirectories*/, bool /*sections*/, bool /*extension*/) {
	// TODO: Implement
	return false;
}

bool Assembly::InitFromMemory(MemAddr /*moduleMemory*/, LoadFlag /*flags*/, const SearchDirs& /*additionalSearchDirectories*/, bool /*sections*/) {
	// TODO: Implement
	return false;
}

bool Assembly::Init(fs::path modulePath, LoadFlag /*flags*/, const SearchDirs& /*additionalSearchDirectories*/, bool /*sections*/) {
	SceKernelModule handle = sceKernelLoadStartModule(modulePath.c_str(), 0, nullptr, 0, nullptr, nullptr);
	switch (handle) {
		case SCE_KERNEL_ERROR_EINVAL:
			_error = "flags or pOpt is invalid";
			return false;
		case SCE_KERNEL_ERROR_ENOENT:
			_error = "File specified in moduleFileName does not exist";
			return false;
		case SCE_KERNEL_ERROR_ENOEXEC:
			_error = "Cannot load because of abnormal file format";
			return false;
		case SCE_KERNEL_ERROR_ENOMEM:
			_error = "Cannot load because it is not possible to allocate memory";
			return false;
		case SCE_KERNEL_ERROR_EACCES:
			_error = "File specified with moduleFileName is placed in a forbidden location";
			return false;
		case SCE_KERNEL_ERROR_EFAULT:
			_error = "moduleFileName points to invalid memory";
			return false;
		case SCE_KERNEL_ERROR_EAGAIN:
			_error = "Cannot load because of insufficient resources";
			return false;
		case SCE_KERNEL_ERROR_ESDKVERSION:
			_error = "Version of the SDK used to build the specified dynamic library is newer than the system software version";
			return false;
		case SCE_KERNEL_ERROR_ESTART:
			_error = "module_start() returned a negative integer";
			return false;
		case SCE_KERNEL_ERROR_ENODYNLIBMEM:
			_error = "Unable to load because there is not enough system memory";
			return false;
		default:
			break;
	}
	_handle = reinterpret_cast<void*>(handle);
	_path = std::move(modulePath);

	// TODO: Implement
	
	return true;
}

MemAddr Assembly::GetVirtualTableByName(std::string_view tableName, bool /*decorated*/) const {
	if (tableName.empty())
		return nullptr;

	// TODO: Implement

	return nullptr;
}

MemAddr Assembly::GetFunctionByName(std::string_view functionName) const noexcept {
	if (!_handle)
		return nullptr;

	if (functionName.empty())
		return nullptr;

	void* address = nullptr;
	[[maybe_unused]] int ret = sceKernelDlsym(reinterpret_cast<SceKernelModule>(_handle), functionName.data(), &address);
#if PLUGIFY_LOGGING
	switch(ret) {
		case SCE_KERNEL_ERROR_ESRCH:
			PL_LOG_VERBOSE("Assembly::GetFunctionByName() - '{}': handle is invalid, or symbol specified in symbol is not exported", functionName);
			break;
		case SCE_KERNEL_ERROR_EFAULT:
			PL_LOG_VERBOSE("Assembly::GetFunctionByName() - '{}': symbol or addrp address is invalid", functionName);
			break;
		default:
			break;
	}
#endif // PLUGIFY_LOGGING
	return address;
}

MemAddr Assembly::GetBase() const noexcept {
	return _handle;
}

namespace plugify {
	int TranslateLoading(LoadFlag) noexcept {
		int sceFlags = 0;
		// No flags
		return sceFlags;
	}

	LoadFlag TranslateLoading(int) noexcept {
		LoadFlag loadFlags = LoadFlag::Default;
		// No flags
		return loadFlags;
	}
}

#endif // PLUGIFY_PLATFORM_ORBIS || PLUGIFY_PLATFORM_PROSPERO
