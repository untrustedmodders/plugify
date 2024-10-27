#if PLUGIFY_PLATFORM_WINDOWS

#include <plugify/assembly.hpp>

#include "os.h"
#include "scope_guard.hpp"

#if PLUGIFY_ARCH_BITS == 64
	const WORD PE_FILE_MACHINE = IMAGE_FILE_MACHINE_AMD64;
	const WORD PE_NT_OPTIONAL_HDR_MAGIC = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
#else
	const WORD PE_FILE_MACHINE = IMAGE_FILE_MACHINE_I386;
	const WORD PE_NT_OPTIONAL_HDR_MAGIC = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
#endif // PLUGIFY_ARCH_BITS

using namespace plugify;

Assembly::~Assembly() {
	if (_handle) {
		[[maybe_unused]] BOOL success = FreeLibrary(static_cast<HMODULE>(_handle));
#if PLUGIFY_LOGGING
		if (!success) {
			DWORD errorCode = GetLastError();
			if (errorCode != 0) {
				LPSTR messageBuffer = NULL;
				DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
											NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, NULL);
				PL_LOG_VERBOSE("Assembly::~Assembly() - '{}': {}", _path.string(), std::string_view(messageBuffer, size));
				LocalFree(messageBuffer);
			}
		}
#endif // PLUGIFY_LOGGING
		_handle = nullptr;
	}
}

static std::wstring GetModulePath(HMODULE hModule) {
	std::wstring modulePath(MAX_PATH, '\0');
	while (true) {
		size_t len = GetModuleFileNameW(hModule, modulePath.data(), static_cast<DWORD>(modulePath.length()));
		if (len == 0) {
			modulePath.clear();
			break;
		}

		if (len < modulePath.length()) {
			modulePath.resize(len);
			break;
		} else
			modulePath.resize(modulePath.length() * 2);
	}

	return modulePath;
}

bool Assembly::InitFromName(std::string_view moduleName, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections, bool extension) {
	if (_handle)
		return false;

	if (moduleName.empty())
		return false;

	fs::path name(moduleName);
	if (!extension && !name.has_extension())
		name += ".dll";

	HMODULE handle = GetModuleHandleW(name.c_str());
	if (!handle)
		return false;

	fs::path modulePath = ::GetModulePath(handle);
	if (modulePath.empty())
		return false;

	if (!Init(modulePath, flags, additionalSearchDirectories, sections))
		return false;

	return true;
}

bool Assembly::InitFromMemory(MemAddr moduleMemory, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections) {
	if (_handle)
		return false;

	if (!moduleMemory)
		return false;

	MEMORY_BASIC_INFORMATION mbi;
	if (!VirtualQuery(moduleMemory, &mbi, sizeof(mbi)))
		return false;

	std::wstring modulePath = ::GetModulePath(reinterpret_cast<HMODULE>(mbi.AllocationBase));
	if (modulePath.empty())
		return false;

	if (!Init(modulePath, flags, additionalSearchDirectories, sections))
		return false;

	return true;
}

bool Assembly::Init(fs::path modulePath, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections) {
	std::vector<DLL_DIRECTORY_COOKIE> dirCookies;
	dirCookies.reserve(additionalSearchDirectories.size());
	for (const auto& directory : additionalSearchDirectories) {
		DLL_DIRECTORY_COOKIE cookie = AddDllDirectory(directory.c_str());
		if (cookie == nullptr)
			continue;
		dirCookies.push_back(cookie);
	}

	auto dirGuard = ScopeGuard([&dirCookies]() {
		for (auto& cookie : dirCookies)
			RemoveDllDirectory(cookie);
	});

	HMODULE hModule = LoadLibraryExW(modulePath.c_str(), nullptr, TranslateLoading(flags));
	if (!hModule) {
		DWORD errorCode = GetLastError();
		if (errorCode != 0) {
			LPSTR messageBuffer = NULL;
			DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
										NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, NULL);
			_error = std::string(messageBuffer, size);
			LocalFree(messageBuffer);
		}
		return false;
	}

	_handle = hModule;
	_path = std::move(modulePath);

	if (flags & LoadFlag::PinInMemory) {
		HMODULE hPinHandle = NULL;
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, reinterpret_cast<LPCWSTR>(hModule), &hPinHandle);
	}

	if (!sections)
		return true;
	
	IMAGE_DOS_HEADER* pDOSHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(hModule);
	IMAGE_NT_HEADERS* pNTHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<uintptr_t>(hModule) + pDOSHeader->e_lfanew);
/*
	IMAGE_FILE_HEADER* pFileHeader = &pNTHeaders->OptionalHeader;
	IMAGE_OPTIONAL_HEADER* pOptionalHeader = &pNTHeaders->OptionalHeader;;

	if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE || pNTHeaders->Signature != IMAGE_NT_SIGNATURE || pOptionalHeader->Magic != PE_NT_OPTIONAL_HDR_MAGIC) {
        _error = "Not a valid DLL file.";
		return false;
	}

	if (pFileHeader->Machine != PE_FILE_MACHINE) {
		_error = "Not a valid DLL file architecture.";
		return false;
	}

	if ((pFileHeader->Characteristics & IMAGE_FILE_DLL) == 0) {
		_error = "DLL file must be a dynamic library.";
		return false;
	}
*/
	const IMAGE_SECTION_HEADER* hSection = IMAGE_FIRST_SECTION(pNTHeaders);// Get first image section.

	// Loop through the sections
	for (WORD i = 0; i < pNTHeaders->FileHeader.NumberOfSections; ++i) {
		const IMAGE_SECTION_HEADER& hCurrentSection = hSection[i]; // Get current section.
		_sections.emplace_back(
			reinterpret_cast<const char*>(hCurrentSection.Name), 
			reinterpret_cast<uintptr_t>(hModule) + hCurrentSection.VirtualAddress, 
			hCurrentSection.SizeOfRawData);// Push back a struct with the section data.
	}

	_executableCode = GetSectionByName(".text");

	return true;
}

MemAddr Assembly::GetVirtualTableByName(std::string_view tableName, bool decorated) const {
	if (tableName.empty())
		return nullptr;

	Assembly::Section runTimeData = GetSectionByName(".data"), readOnlyData = GetSectionByName(".rdata");
	if (!runTimeData.IsValid() || !readOnlyData.IsValid())
		return nullptr;

	std::string decoratedTableName(decorated ? tableName : ".?AV" + std::string(tableName) + "@@");
	std::string mask(decoratedTableName.length() + 1, 'x');

	MemAddr typeDescriptorName = FindPattern(decoratedTableName.data(), mask, nullptr, &runTimeData);
	if (!typeDescriptorName)
		return nullptr;

	MemAddr rttiTypeDescriptor = typeDescriptorName.Offset(-0x10);
	const uintptr_t rttiTDRva = rttiTypeDescriptor - GetBase();// The RTTI gets referenced by a 4-Byte RVA address. We need to scan for that address.

	MemAddr reference; // Get reference typeinfo in vtable
	while ((reference = FindPattern(&rttiTDRva, "xxxx", reference, &readOnlyData))) {
		// Check if we got a RTTI Object Locator for this reference by checking if -0xC is 1, which is the 'signature' field which is always 1 on x64.
		// Check that offset of this vtable is 0
		if (reference.Offset(-0xC).GetValue<int32_t>() == 1 && reference.Offset(-0x8).GetValue<int32_t>() == 0) {
			MemAddr referenceOffset = reference.Offset(-0xC);
			MemAddr rttiCompleteObjectLocator = FindPattern(&referenceOffset, "xxxxxxxx", nullptr, &readOnlyData);
			if (rttiCompleteObjectLocator)
				return rttiCompleteObjectLocator.Offset(0x8);
		}

		reference.OffsetSelf(0x4);
	}

	return nullptr;
}

MemAddr Assembly::GetFunctionByName(std::string_view functionName) const noexcept {
	if (!_handle)
		return nullptr;

	if (functionName.empty())
		return nullptr;

	FARPROC pAddress = GetProcAddress(static_cast<HMODULE>(_handle), functionName.data());
#if PLUGIFY_LOGGING
	if (!pAddress) {
		DWORD errorCode = GetLastError();
		if (errorCode != 0) {
			LPSTR messageBuffer = NULL;
			DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
										NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, NULL);
			PL_LOG_VERBOSE("Assembly::GetFunctionByName() - '{}': {}", functionName, std::string_view(messageBuffer, size));
			LocalFree(messageBuffer);
		}
	}
#endif // PLUGIFY_LOGGING
	return pAddress;
}

MemAddr Assembly::GetBase() const noexcept {
	return _handle;
}

namespace plugify {
	int TranslateLoading(LoadFlag flags) noexcept {
		int winFlags = 0;
		if (flags & LoadFlag::DontResolveDllReferences) winFlags |= DONT_RESOLVE_DLL_REFERENCES;
		if (flags & LoadFlag::AlteredSearchPath) winFlags |= LOAD_WITH_ALTERED_SEARCH_PATH;
		if (flags & LoadFlag::AsDatafile) winFlags |= LOAD_LIBRARY_AS_DATAFILE;
		if (flags & LoadFlag::AsDatafileExclusive) winFlags |= LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE;
		if (flags & LoadFlag::AsImageResource) winFlags |= LOAD_LIBRARY_AS_IMAGE_RESOURCE;
		if (flags & LoadFlag::SearchApplicationDir) winFlags |= LOAD_LIBRARY_SEARCH_APPLICATION_DIR;
		if (flags & LoadFlag::SearchDefaultDirs) winFlags |= LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
		if (flags & LoadFlag::SearchDllLoadDir) winFlags |= LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR;
		if (flags & LoadFlag::SearchSystem32) winFlags |= LOAD_LIBRARY_SEARCH_SYSTEM32;
		if (flags & LoadFlag::SearchUserDirs) winFlags |= LOAD_LIBRARY_SEARCH_USER_DIRS;
		if (flags & LoadFlag::RequireSignedTarget) winFlags |= LOAD_LIBRARY_REQUIRE_SIGNED_TARGET;
		if (flags & LoadFlag::IgnoreAuthzLevel) winFlags |= LOAD_IGNORE_CODE_AUTHZ_LEVEL;
#ifdef LOAD_LIBRARY_SAFE_CURRENT_DIRS
		if (flags & LoadFlag::SafeCurrentDirs) winFlags |= LOAD_LIBRARY_SAFE_CURRENT_DIRS;
#endif // LOAD_LIBRARY_SAFE_CURRENT_DIRS
		return winFlags;
	}

	LoadFlag TranslateLoading(int flags) noexcept {
		LoadFlag loadFlags = LoadFlag::Default;
		if (flags & DONT_RESOLVE_DLL_REFERENCES) loadFlags = loadFlags | LoadFlag::DontResolveDllReferences;
		if (flags & LOAD_WITH_ALTERED_SEARCH_PATH) loadFlags = loadFlags | LoadFlag::AlteredSearchPath;
		if (flags & LOAD_LIBRARY_AS_DATAFILE) loadFlags = loadFlags | LoadFlag::AsDatafile;
		if (flags & LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE) loadFlags = loadFlags | LoadFlag::AsDatafileExclusive;
		if (flags & LOAD_LIBRARY_AS_IMAGE_RESOURCE) loadFlags = loadFlags | LoadFlag::AsImageResource;
		if (flags & LOAD_LIBRARY_SEARCH_APPLICATION_DIR) loadFlags = loadFlags | LoadFlag::SearchApplicationDir;
		if (flags & LOAD_LIBRARY_SEARCH_DEFAULT_DIRS) loadFlags = loadFlags | LoadFlag::SearchDefaultDirs;
		if (flags & LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR) loadFlags = loadFlags | LoadFlag::SearchDllLoadDir;
		if (flags & LOAD_LIBRARY_SEARCH_SYSTEM32) loadFlags = loadFlags | LoadFlag::SearchSystem32;
		if (flags & LOAD_LIBRARY_SEARCH_USER_DIRS) loadFlags = loadFlags | LoadFlag::SearchUserDirs;
		if (flags & LOAD_LIBRARY_REQUIRE_SIGNED_TARGET) loadFlags = loadFlags | LoadFlag::RequireSignedTarget;
		if (flags & LOAD_IGNORE_CODE_AUTHZ_LEVEL) loadFlags = loadFlags | LoadFlag::IgnoreAuthzLevel;
#ifdef LOAD_LIBRARY_SAFE_CURRENT_DIRS
		if (flags & LOAD_LIBRARY_SAFE_CURRENT_DIRS) loadFlags = loadFlags | LoadFlag::SafeCurrentDirs;
#endif // LOAD_LIBRARY_SAFE_CURRENT_DIRS
		return loadFlags;
	}
}

#endif // PLUGIFY_PLATFORM_WINDOWS
