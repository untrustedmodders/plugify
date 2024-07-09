#if PLUGIFY_PLATFORM_APPLE

#include <plugify/assembly.h>
#include "os.h"

using namespace plugify;

static constexpr int DEFAULT_LIBRARY_LOAD_FLAGS = RTLD_LAZY;

Assembly::~Assembly() {
	if (_handle) {
		dlclose(_handle);
		_handle = nullptr;
	}
}

bool Assembly::InitFromName(std::string_view /*moduleName*/, int /*flags*/, bool /*extension*/, bool /*sections*/) {
	// TODO: Implement
	return false;
}

bool Assembly::InitFromMemory(MemAddr moduleMemory, int flags, bool sections) {
	if (_handle)
		return false;

	if (!moduleMemory)
		return false;

	Dl_info info;
	if (!dladdr(moduleMemory, &info) || !info.dli_fbase || !info.dli_fname)
		return false;

	if (!Init(info.dli_fname, flags, sections))
		return false;

	return true;
}

bool Assembly::Init(const fs::path& modulePath, int flags, bool /*sections*/) {
	void* handle = dlopen(modulePath.c_str(), flags != -1 ? flags : DEFAULT_LIBRARY_LOAD_FLAGS);
	if (!handle) {
		_path = dlerror();
		return false;
	}

	_handle = handle;
	_path = modulePath.string();

	return true;
	// TODO: Implement

	/*if (!sections)
		return true;

	link_map* lmap;
	if (dlinfo(handle, RTLD_DI_LINKMAP, &lmap) != 0) {
		_path = "Failed to retrieve dynamic linker information using dlinfo.";
		dlclose(handle);
		return false;
	}

	int fd = open(lmap->l_name, O_RDONLY);
	if (fd == -1) {
		_path = "Failed to open the shared object file.";
		dlclose(handle);
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
				if (*(strTab + shdr->sh_name) == '\0')
					continue;

				_sections.emplace_back(strTab + shdr->sh_name, static_cast<uintptr_t>(lmap->l_addr + shdr->sh_addr), shdr->sh_size);
			}

			munmap(map, static_cast<size_t>(st.st_size));
		}
	}

	close(fd);

	_executableCode = GetSectionByName(".text");

	return true;*/
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

	for (const auto& sectionName: {std::string_view(".data.rel.ro"), std::string_view(".data.rel.ro.local")}) {
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

MemAddr Assembly::GetFunctionByName(std::string_view functionName) const {
	if (!_handle)
		return nullptr;

	if (functionName.empty())
		return nullptr;

	return dlsym(_handle, functionName.data());
}

MemAddr Assembly::GetBase() const {
	return static_cast<link_map*>(_handle)->l_addr;
}

#endif