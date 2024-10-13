#if PLUGIFY_PLATFORM_LINUX

#include <plugify/assembly.h>

#include "os.h"

#if PLUGIFY_ARCH_BITS == 64
	const unsigned char ELF_CLASS = ELFCLASS64;
	const uint16_t ELF_MACHINE = EM_X86_64;
#else
	const unsigned char ELF_CLASS = ELFCLASS32;
	const uint16_t ELF_MACHINE = EM_386;
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

bool Assembly::InitFromName(std::string_view moduleName, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections, bool extension) {
	if (_handle)
		return false;

	if (moduleName.empty())
		return false;

	std::string name(moduleName);
	if (!extension && !(name.find(".so.") != std::string::npos || name.find_last_of(".so") == name.length() - 3))
		name += ".so";

	struct dl_data {
		ElfW(Addr) addr;
		const char* moduleName;
		const char* modulePath;
	} dldata{0, name.c_str(), {}};

	dl_iterate_phdr([](dl_phdr_info* info, size_t /* size */, void* data) {
		dl_data* _dldata = static_cast<dl_data*>(data);

		if (std::strstr(info->dlpi_name, _dldata->moduleName) != nullptr) {
			_dldata->addr = info->dlpi_addr;
			_dldata->modulePath = info->dlpi_name;
		}

		return 0;
	}, &dldata);

	if (!dldata.addr)
		return false;

	if (!Init(dldata.modulePath, flags, additionalSearchDirectories, sections))
		return false;

	return true;
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
	// Cannot set LD_LIBRARY_PATH at runtime, so use rpath flag

	void* handle = dlopen(modulePath.c_str(), TranslateLoading(flags));
	if (!handle) {
		_error = dlerror();
		return false;
	}

	_handle = handle;
	_path = std::move(modulePath);

	if (!sections)
		return true;

	link_map* lmap;
	if (dlinfo(handle, RTLD_DI_LINKMAP, &lmap) != 0) {
		_error = "Failed to retrieve dynamic linker information using dlinfo.";
		return false;
	}
/*
	ElfW(Phdr) file = lmap->l_addr;

	if (memcmp(ELFMAG, file->e_ident, SELFMAG) != 0) {
		_error = "Not a valid ELF file.";
		return false;
	}

	if (file->e_ident[EI_VERSION] != EV_CURRENT) {
		_error = "Not a valid ELF file version.";
		return false;
	}

	if (file->e_ident[EI_CLASS] != ELF_CLASS || file->e_machine != ELF_MACHINE || file->e_ident[EI_DATA] != ELFDATA2LSB) {
		_error = "Not a valid ELF file architecture.";
		return false;
	}

	if (file->e_type != ET_DYN) {
		_error = "ELF file must be a dynamic library.";
		return false;
	}
*/
	int fd = open(lmap->l_name, O_RDONLY);
	if (fd == -1) {
		_error = "Failed to open the shared object file.";
		return false;
	}

	struct stat st{};
	if (fstat(fd, &st) == 0) {
		void* map = mmap(nullptr, static_cast<size_t>(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
		if (map != MAP_FAILED) {
			ElfW(Ehdr)* ehdr = static_cast<ElfW(Ehdr)*>(map);
			ElfW(Shdr)* shdrs = reinterpret_cast<ElfW(Shdr)*>(reinterpret_cast<uintptr_t>(ehdr) + ehdr->e_shoff);
			const char* strTab = reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(ehdr) + shdrs[ehdr->e_shstrndx].sh_offset);

			// Loop through the sections.
			for (auto i = 0; i < ehdr->e_shnum; ++i) {
				ElfW(Shdr)* shdr = reinterpret_cast<ElfW(Shdr)*>(reinterpret_cast<uintptr_t>(shdrs) + static_cast<uintptr_t>(i * ehdr->e_shentsize));
				if (*(strTab + shdr->sh_name) == 0)
					continue;

				_sections.emplace_back(strTab + shdr->sh_name, lmap->l_addr + shdr->sh_addr, shdr->sh_size);
			}

			munmap(map, static_cast<size_t>(st.st_size));
		}
	}

	close(fd);

	_executableCode = GetSectionByName(".text");

	return true;
}

MemAddr Assembly::GetVirtualTableByName(std::string_view tableName, bool decorated) const {
	if (tableName.empty())
		return nullptr;

	Assembly::Section readOnlyData = GetSectionByName(".rodata"), readOnlyRelocations = GetSectionByName(".data.rel.ro");
	if (!readOnlyData.IsValid() || !readOnlyRelocations.IsValid())
		return nullptr;

	std::string decoratedTableName(decorated ? tableName : std::to_string(tableName.length()) + std::string(tableName));
	std::string mask(decoratedTableName.length() + 1, 'x');

	MemAddr typeInfoName = FindPattern(decoratedTableName.data(), mask, nullptr, &readOnlyData);
	if (!typeInfoName)
		return nullptr;

	MemAddr referenceTypeName = FindPattern(&typeInfoName, "xxxxxxxx", nullptr, &readOnlyRelocations);// Get reference to type name.
	if (!referenceTypeName)
		return nullptr;

	MemAddr typeInfo = referenceTypeName.Offset(-0x8);// Offset -0x8 to typeinfo.

	for (const auto& sectionName : {std::string_view(".data.rel.ro"), std::string_view(".data.rel.ro.local")}) {
		Assembly::Section section = GetSectionByName(sectionName);
		if (!section.IsValid())
			continue;

		MemAddr reference;// Get reference typeinfo in vtable
		while ((reference = FindPattern(&typeInfo, "xxxxxxxx", reference, &section))) {
			// Offset to this.
			if (reference.Offset(-0x8).GetValue<int64_t>() == 0) {
				return reference.Offset(0x8);
			}

			reference.OffsetSelf(0x8);
		}
	}

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
	return static_cast<link_map*>(_handle)->l_addr;
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

#endif // PLUGIFY_PLATFORM_LINUX
