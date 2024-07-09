#include <plugify/assembly.h>

#include <array>
#include <cmath>
#include <cstring>

#if defined(__GNUC__) && !defined(__clang__) && !defined(NDEBUG)
#undef __OPTIMIZE__
#endif
#include <emmintrin.h>

using namespace plugify;
namespace fs = std::filesystem;

Assembly::Assembly(std::string_view moduleName, int flags, bool sections) : _handle{nullptr} {
	InitFromName(moduleName, flags, sections);
}

Assembly::Assembly(const MemAddr moduleMemory, int flags, bool sections) : _handle{nullptr} {
	InitFromMemory(moduleMemory, flags, sections);
}

Assembly::Assembly(const std::filesystem::path& modulePath, int flags, bool sections) : _handle{nullptr} {
	Init(modulePath, flags, sections);
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

	return nullptr;
}

MemAddr Assembly::FindPattern(std::string_view pattern, MemAddr startAddress, Section* moduleSection) const {
	const std::pair patternInfo = PatternToMaskedBytes(pattern);
	return FindPattern(patternInfo.first.data(), patternInfo.second, startAddress, moduleSection);
}

Assembly::Section Assembly::GetSectionByName(std::string_view sectionName) const {
	for (const Section& section: _sections) {
		if (section.name == sectionName)
			return section;
	}

	return {};
}

void* Assembly::GetHandle() const {
	return _handle;
}

std::string_view Assembly::GetPath() const {
	return _path;
}

std::string_view Assembly::GetName() const {
	std::string_view modulePath(_path);
	return modulePath.substr(modulePath.find_last_of("/\\") + 1);
}

std::string_view Assembly::GetError() const {
	return !IsValid() ? _path : std::string_view("");
}

#if PLUGIFY_SEPARATE_SOURCE_FILES
#if PLUGIFY_PLATFORM_WINDOWS
#include "assembly_windows.cpp"
#elif PLUGIFY_PLATFORM_LINUX
#include "assembly_linux.cpp"
#elif PLUGIFY_PLATFORM_APPLE
#include "assembly_apple.cpp"
#else
#error "Unsupported platform"
#endif
#endif