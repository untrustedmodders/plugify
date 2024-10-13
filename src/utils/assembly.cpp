#include <plugify/assembly.h>

#include <array>
#include <cmath>
#include <cstring>

#if !PLUGIFY_ARCH_ARM
#if PLUGIFY_COMPILER_GCC && !PLUGIFY_COMPILER_CLANG && !defined(NDEBUG)
#undef __OPTIMIZE__
#endif // !defined(NDEBUG)
#include <emmintrin.h>
#endif // !PLUGIFY_USE_ARM

using namespace plugify;
namespace fs = std::filesystem;

Assembly::Assembly(std::string_view moduleName, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections) : _handle{nullptr} {
	InitFromName(moduleName, flags, additionalSearchDirectories, sections);
}

Assembly::Assembly(const MemAddr moduleMemory, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections) : _handle{nullptr} {
	InitFromMemory(moduleMemory, flags, additionalSearchDirectories, sections);
}

Assembly::Assembly(const fs::path& modulePath, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections) : _handle{nullptr} {
	Init(modulePath, flags, additionalSearchDirectories, sections);
}

std::pair<std::vector<uint8_t>, std::string> Assembly::PatternToMaskedBytes(std::string_view input) {
	char* pPatternStart = const_cast<char*>(input.data());
	char* pPatternEnd = pPatternStart + input.size();
	std::vector<uint8_t> bytes;
	std::string mask;

	for (char* pCurrentByte = pPatternStart; pCurrentByte < pPatternEnd; ++pCurrentByte) {
		if (*pCurrentByte == '?') {
			++pCurrentByte;
			if (*pCurrentByte == '?') {
				++pCurrentByte;// Skip double wildcard.
			}

			bytes.push_back(0);// Push the byte back as invalid.
			mask += '?';
		} else {
			bytes.push_back(static_cast<uint8_t>(strtoul(pCurrentByte, &pCurrentByte, 16)));
			mask += 'x';
		}
	}

	return std::make_pair(std::move(bytes), std::move(mask));
}

MemAddr Assembly::FindPattern(MemAddr pattern, std::string_view mask, MemAddr startAddress, Section* moduleSection) const {
	const uint8_t* pPattern = pattern.RCast<const uint8_t*>();
	const Section* section = moduleSection ? moduleSection : &_executableCode;
	if (!section->IsValid())
		return nullptr;

	const uintptr_t base = section->base;
	const size_t size = section->size;

	const size_t maskLen = mask.length();
	const uint8_t* pData = reinterpret_cast<uint8_t*>(base);
	const uint8_t* pEnd = pData + size - maskLen;

	if (startAddress) {
		const uint8_t* pStartAddress = startAddress.RCast<uint8_t*>();
		if (pData > pStartAddress || pStartAddress > pEnd)
			return nullptr;

		pData = pStartAddress;
	}

#if !PLUGIFY_ARCH_ARM
	std::array<int, 64> masks = {};// 64*16 = enough masks for 1024 bytes.
	const uint8_t numMasks = static_cast<uint8_t>(std::ceil(static_cast<float>(maskLen) / 16.f));

	for (uint8_t i = 0; i < numMasks; ++i) {
		for (int8_t j = static_cast<int8_t>(std::min<size_t>(maskLen - i * 16, 16)) - 1; j >= 0; --j) {
			if (mask[static_cast<size_t>(i * 16 + j)] == 'x') {
				masks[i] |= 1 << j;
			}
		}
	}

	const __m128i xmm1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pPattern));
	__m128i xmm2, xmm3, msks;
	for (; pData != pEnd; _mm_prefetch(reinterpret_cast<const char*>(++pData + 64), _MM_HINT_NTA)) {
		xmm2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pData));
		msks = _mm_cmpeq_epi8(xmm1, xmm2);
		if ((_mm_movemask_epi8(msks) & masks[0]) == masks[0]) {
			bool found = true;
			for (uint8_t i = 1; i < numMasks; ++i) {
				xmm2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>((pData + i * 16)));
				xmm3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>((pPattern + i * 16)));
				msks = _mm_cmpeq_epi8(xmm2, xmm3);
				if ((_mm_movemask_epi8(msks) & masks[i]) != masks[i]) {
					found = false;
					break;
				}
			}

			if (found)
				return pData;
		}
	}
#else
	for (; pData != pEnd; ++pData) {
		bool found = false;

		for (size_t i = 0; i < maskLen; ++i) {
			if (mask[i] == 'x' || pPattern[i] == *(pData + i)) {
				found = true;
			} else {
				found = false;
				break;
			}
		}

		if (found)
			return pData;
	}
#endif // !PLUGIFY_ARCH_ARM
	return nullptr;
}

MemAddr Assembly::FindPattern(std::string_view pattern, MemAddr startAddress, Section* moduleSection) const {
	const std::pair patternInfo = PatternToMaskedBytes(pattern);
	return FindPattern(patternInfo.first.data(), patternInfo.second, startAddress, moduleSection);
}

Assembly::Section Assembly::GetSectionByName(std::string_view sectionName) const noexcept {
	for (const Section& section : _sections) {
		if (section.name == sectionName)
			return section;
	}

	return {};
}

void* Assembly::GetHandle() const noexcept {
	return _handle;
}

const fs::path& Assembly::GetPath() const noexcept {
	return _path;
}

const std::string& Assembly::GetError() const noexcept {
	return _error;
}

#if PLUGIFY_SEPARATE_SOURCE_FILES
#if PLUGIFY_PLATFORM_WINDOWS
#include "assembly_windows.cpp"
#elif PLUGIFY_PLATFORM_LINUX
#include "assembly_linux.cpp"
#elif PLUGIFY_PLATFORM_APPLE
#include "assembly_apple.cpp"
#elif PLUGIFY_PLATFORM_ORBIS || PLUGIFY_PLATFORM_PROSPERO
#include "assembly_playstation.cpp"
#elif PLUGIFY_PLATFORM_SWITCH
#include "assembly_switch.cpp"
#else
#error "Unsupported platform"
#endif
#endif // PLUGIFY_SEPARATE_SOURCE_FILES
