#pragma once

#if !PLUGIFY_ARCH_ARM
#if PLUGIFY_COMPILER_GCC && !PLUGIFY_COMPILER_CLANG && !defined(NDEBUG)
#undef __OPTIMIZE__
#endif // !defined(NDEBUG)
#include <emmintrin.h>

#if PLUGIFY_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif  // PLUGIFY_COMPILER_GCC

#if PLUGIFY_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif  // PLUGIFY_COMPILER_CLANG

#if PLUGIFY_COMPILER_MSVC
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif // PLUGIFY_COMPILER_MSVC
#endif // !PLUGIFY_USE_ARM

namespace plugify {
	union Digest {
		std::array<uint8_t, 32> h;
#if !PLUGIFY_ARCH_ARM
		struct {
			__m128i h0123; // h0:h1:h2:h3
			__m128i h4567; // h4:h5:h6:h7
		};
#endif // !PLUGIFY_ARCH_ARM
	};

	class Sha256 {
	public:
		Sha256();
		//static constexpr size_t blocklen = 64;
		//static constexpr size_t digestlen = 32;

		void clear();
		void update(std::span<const uint8_t>);

		Digest digest();

		static std::string ToString(const Digest& digest);

		// Intermediate hash
		union {
			std::array<uint32_t, 8> _h;
#if !PLUGIFY_ARCH_ARM
			struct {
				__m128i _h0145; // h0:h1:h4:h5
				__m128i _h2367; // h2:h3:h6:h7
			};
#endif // !PLUGIFY_ARCH_ARM
		};

	private:
		std::array<uint8_t, 64> _buf;
		size_t _len;
	};
}

#if !PLUGIFY_ARCH_ARM
#if PLUGIFY_COMPILER_CLANG
#pragma clang diagnostic pop
#endif  // PLUGIFY_COMPILER_CLANG

#if PLUGIFY_COMPILER_GCC
#pragma GCC diagnostic pop
#endif  // PLUGIFY_COMPILER_GCC

#if PLUGIFY_COMPILER_MSVC
#pragma warning( pop )
#endif // PLUGIFY_COMPILER_MSVC
#endif // !PLUGIFY_USE_ARM