#include "os.h"
#include <plugify/mem_accessor.hpp>
#include <plugify/mem_protector.hpp>

#include <cstring>

using namespace plugify;

#define MEMORY_ROUND(_numToRound_, _multiple_) \
	(_numToRound_ & (((size_t)-1) ^ (_multiple_ - 1)))

// Round _numToRound_ to the next higher _multiple_
#define MEMORY_ROUND_UP(_numToRound_, _multiple_) \
	((_numToRound_ + (_multiple_ - 1)) & (((size_t)-1) ^ (_multiple_ - 1)))

#if PLUGIFY_PLATFORM_WINDOWS

bool MemAccessor::MemCopy(MemAddr dest, MemAddr src, size_t size) {
	std::memcpy(dest, src, size);
	return true;
}

bool MemAccessor::SafeMemCopy(MemAddr dest, MemAddr src, size_t size, size_t& written) noexcept {
	written = 0;
	return WriteProcessMemory(GetCurrentProcess(), dest, src, (SIZE_T)size, (PSIZE_T)&written);
}

bool MemAccessor::SafeMemRead(MemAddr src, MemAddr dest, size_t size, size_t& read) noexcept {
	HANDLE process = GetCurrentProcess();
	read = 0;

	if (ReadProcessMemory(process, src, dest, (SIZE_T)size, (PSIZE_T)&read) && read > 0)
		return true;

	// Tries to read again on a partial copy, but limited by the end of the memory region
	if (GetLastError() == ERROR_PARTIAL_COPY) {
		MEMORY_BASIC_INFORMATION info;
		if (VirtualQueryEx(process, src, &info, sizeof(info)) != 0) {
			uintptr_t end = reinterpret_cast<uintptr_t>(info.BaseAddress) + info.RegionSize;
			if (src + size > end)
				return ReadProcessMemory(process, src, dest, static_cast<SIZE_T>(end - src), static_cast<PSIZE_T>(&read)) && read > 0;
		}
	}
	return false;
}

ProtFlag MemAccessor::MemProtect(MemAddr dest, size_t size, ProtFlag prot, bool& status)  {
	DWORD orig;
	status = VirtualProtect(dest, static_cast<SIZE_T>(size), TranslateProtection(prot), &orig) != 0;
	return TranslateProtection(static_cast<int>(prot));
}

#elif PLUGIFY_PLATFORM_LINUX

#include <fstream>

struct region_t {
	uintptr_t start;
	uintptr_t end;
	ProtFlag prot;
};

static region_t get_region_from_addr(uintptr_t addr) {
	region_t res{};

	std::ifstream f("/proc/self/maps");
	std::string s;
	while (std::getline(f, s)) {
		if (!s.empty() && s.find("vdso") == std::string::npos && s.find("vsyscall") == std::string::npos) {
			char* strend = &s[0];
			uintptr_t start = strtoul(strend  , &strend, 16);
			uintptr_t end   = strtoul(strend+1, &strend, 16);
			if (start != 0 && end != 0 && start <= addr && addr < end) {
				res.start = start;
				res.end = end;

				++strend;
				if (strend[0] == 'r')
					res.prot = res.prot | ProtFlag::R;
	
				if (strend[1] == 'w')
					res.prot = res.prot | ProtFlag::W;
	
				if (strend[2] == 'x')
					res.prot = res.prot | ProtFlag::X;

				if (res.prot == ProtFlag::UNSET)
					res.prot = ProtFlag::N;

				break;
			}
		}
	}
	return res;
}

bool MemAccessor::MemCopy(MemAddr dest, MemAddr src, size_t size) {
	std::memcpy(dest, src, size);
	return true;
}

bool MemAccessor::SafeMemCopy(MemAddr dest, MemAddr src, size_t size, size_t& written) noexcept {
	region_t region_infos = get_region_from_addr(src);
	
	// Make sure that the region we query is writable
	if (!(region_infos.prot & ProtFlag::W))
		return false;
	
	size = std::min<uintptr_t>(region_infos.end - src, size);
	
	std::memcpy(dest, src, size);
	written = size;

	return true;
}

bool MemAccessor::SafeMemRead(MemAddr src, MemAddr dest, size_t size, size_t& read) noexcept {
	region_t region_infos = get_region_from_addr(src);
	
	// Make sure that the region we query is readable
	if (!(region_infos.prot & ProtFlag::R))
		return false;

	size = std::min<uintptr_t>(region_infos.end - src, size);

	std::memcpy(dest, src, size);
	read = size;

	return true;
}

ProtFlag MemAccessor::MemProtect(MemAddr dest, size_t size, ProtFlag prot, bool& status) {
	static auto pageSize = static_cast<size_t>(sysconf(_SC_PAGESIZE));
	region_t regionInfo = get_region_from_addr(dest);
	uintptr_t alignedDest = MEMORY_ROUND(dest, pageSize);
	uintptr_t alignedSize = MEMORY_ROUND_UP(size, pageSize);
	status = mprotect(reinterpret_cast<void*>(alignedDest), alignedSize, TranslateProtection(prot)) == 0;
	return regionInfo.prot;
}

#elif PLUGIFY_PLATFORM_APPLE

bool MemAccessor::MemCopy(MemAddr dest, MemAddr src, size_t size)  {
	std::memcpy(dest, src, size);
	return true;
}

bool MemAccessor::SafeMemCopy(MemAddr dest, MemAddr src, size_t size, size_t& written) noexcept {
	bool res = std::memcpy(dest, src, size) != nullptr;
	if (res)
		written = size;
	else
		written = 0;

	return res;
}

bool MemAccessor::SafeMemRead(MemAddr src, MemAddr dest, size_t size, size_t& read) noexcept {
	bool res = std::memcpy(dest, src, size) != nullptr;
	if (res)
		read = size;
	else
		read = 0;

	return res;
}

ProtFlag MemAccessor::MemProtect(MemAddr dest, size_t size, ProtFlag prot, bool& status) {
	static auto pageSize = static_cast<size_t>(sysconf(_SC_PAGESIZE));
	status = mach_vm_protect(mach_task_self(), static_cast<mach_vm_address_t>(MEMORY_ROUND(dest, pageSize)), static_cast<mach_vm_size_t>(MEMORY_ROUND_UP(size, pageSize)), FALSE, TranslateProtection(prot)) == KERN_SUCCESS;
	return ProtFlag::R | ProtFlag::X;
}

#endif
