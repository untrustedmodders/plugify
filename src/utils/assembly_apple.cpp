#if PLUGIFY_PLATFORM_APPLE

#include <plugify/assembly.h>

#include "os.h"
	
#if PLUGIFY_ARCH_BITS == 64
	typedef struct mach_header_64 MachHeader;
	typedef struct segment_command_64 MachSegment;
	typedef struct section_64 MachSection;
	const uint32_t MACH_MAGIC = MH_MAGIC_64;
	const uint32_t MACH_LOADCMD_SEGMENT = LC_SEGMENT_64;
	const cpu_type_t MACH_CPU_TYPE = CPU_TYPE_X86_64;
	const cpu_subtype_t MACH_CPU_SUBTYPE = CPU_SUBTYPE_X86_64_ALL;
#else
	typedef struct mach_header MachHeader;
	typedef struct segment_command MachSegment;
	typedef struct section MachSection;
	const uint32_t MACH_MAGIC = MH_MAGIC;
	const uint32_t MACH_LOADCMD_SEGMENT = LC_SEGMENT;
	const cpu_type_t MACH_CPU_TYPE = CPU_TYPE_I386;
	const cpu_subtype_t MACH_CPU_SUBTYPE = CPU_SUBTYPE_I386_ALL;
#endif // PLUGIFY_ARCH_BITS

using namespace plugify;

Assembly::~Assembly() {
	if (_handle) {
		[[maybe_unused]] int error = dlclose(_handle);
#if PLUGIFY_LOGGING
		if (error) {
			PL_LOG_VERBOSE("Assembly::~Assembly() - '{}': {}", _path.c_str(), dlerror());
		}
#endif // PLUGIFY_LOGGING
		_handle = nullptr;
	}
}

bool Assembly::InitFromName(std::string_view /*moduleName*/, LoadFlag /*flags*/, const SearchDirs& /*additionalSearchDirectories*/, bool /*extension*/, bool /*sections*/) {
	// TODO: Implement
	return false;
}

bool Assembly::InitFromMemory(MemAddr moduleMemory, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections) {
	if (_handle)
		return false;

	if (!moduleMemory)
		return false;

	Dl_info info;
	if (!dladdr(moduleMemory, &info) || !info.dli_fbase || !info.dli_fname)
		return false;

	if (!Init(info.dli_fname, flags, additionalSearchDirectories, sections))
		return false;

	return true;
}

bool Assembly::Init(fs::path modulePath, LoadFlag flags, const SearchDirs& /*additionalSearchDirectories*/, bool sections) {
	// Cannot set LYLD_LIBRARY_PATH at runtime, so use rpath flag

	void* handle = dlopen(modulePath.c_str(), TranslateLoading(flags));
	if (!handle) {
		_error = dlerror();
		return false;
	}

	_handle = handle;
	_path = std::move(modulePath);

	if (!sections)
		return true;

    const MachHeader* header = reinterpret_cast<const MachHeader*>(_handle);
/*
    if (header->magic != MACH_MAGIC) {
        _error = "Not a valid Mach-O file.";
        return false;
    }
	
	if (header->cputype != MACH_CPU_TYPE || header->cpusubtype != MACH_CPU_SUBTYPE) {
        _error = "Not a valid Mach-O file architecture.";
		return false;
	}

	if (header->filetype != MH_DYLIB) {
        _error = "Mach-O file must be a dynamic library.";
		return false;
	}
*/
    const load_command* cmd = reinterpret_cast<const load_command*>(reinterpret_cast<uintptr_t>(header) + sizeof(MachHeader));
    for (uint32_t i = 0; i < header->ncmds; ++i) {
        if (cmd->cmd == MACH_LOADCMD_SEGMENT) {
            const MachSegment* seg = reinterpret_cast<const MachSegment*>(cmd);
            const MachSection* sec = reinterpret_cast<const MachSection*>(reinterpret_cast<uintptr_t>(seg) + sizeof(MachSegment));

            for (uint32_t j = 0; j < seg->nsects; ++j) {
                const MachSection& section = sec[j];
				_sections.emplace_back(
                    section.sectname,
                    reinterpret_cast<uintptr_t>(_handle) + section.addr,
                    section.size
                );
            }
        }
        cmd = reinterpret_cast<const load_command*>(reinterpret_cast<uintptr_t>(cmd) + cmd->cmdsize);
    }
	
	_executableCode = GetSectionByName("__TEXT");

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

	void* address = dlsym(_handle, functionName.data());
#if PLUGIFY_LOGGING
	if (!address) {
		PL_LOG_VERBOSE("Assembly::GetFunctionByName() - '{}': {}", functionName, dlerror());
	}
#endif // PLUGIFY_LOGGING
	return address;
}

MemAddr Assembly::GetBase() const noexcept {
	return _handle;
}

namespace plugify {
	int TranslateLoading(LoadFlag flags) noexcept {
		int unixFlags = 0;
		if (flags & LoadFlag::Lazy) unixFlags |= RTLD_LAZY;
		if (flags & LoadFlag::Now) unixFlags |= RTLD_NOW;
		if (flags & LoadFlag::Global) unixFlags |= RTLD_GLOBAL;
		if (flags & LoadFlag::Local) unixFlags |= RTLD_LOCAL;
		if (flags & LoadFlag::Nodelete) unixFlags |= RTLD_NODELETE;
		if (flags & LoadFlag::Noload) unixFlags |= RTLD_NOLOAD;
#ifdef RTLD_DEEPBIND
		if (flags & LoadFlag::Deepbind) unixFlags |= RTLD_DEEPBIND;
#endif // RTLD_DEEPBIND
		return unixFlags;
	}

	LoadFlag TranslateLoading(int flags) noexcept {
		LoadFlag loadFlags = LoadFlag::Default;
		if (flags & RTLD_LAZY) loadFlags = loadFlags | LoadFlag::Lazy;
		if (flags & RTLD_NOW) loadFlags = loadFlags | LoadFlag::Now;
		if (flags & RTLD_GLOBAL) loadFlags = loadFlags | LoadFlag::Global;
		if (flags & RTLD_LOCAL) loadFlags = loadFlags | LoadFlag::Local;
		if (flags & RTLD_NODELETE) loadFlags = loadFlags | LoadFlag::Nodelete;
		if (flags & RTLD_NOLOAD) loadFlags = loadFlags | LoadFlag::Noload;
#ifdef RTLD_DEEPBIND
		if (flags & RTLD_DEEPBIND) loadFlags = loadFlags | LoadFlag::Deepbind;
#endif // RTLD_DEEPBIND
		return loadFlags;
	}
}

#endif // PLUGIFY_PLATFORM_APPLE
