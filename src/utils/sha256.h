#pragma once

#if defined(__GNUC__) && !defined(__clang__) && !defined(NDEBUG)
#undef __OPTIMIZE__
#endif
#include <emmintrin.h>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif  // defined(__GNUC__)

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif  // defined(__clang__)

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif // defined(_MSC_VER)

namespace plugify {
	union Digest {
		std::array<uint8_t, 32> h;
		struct {
			__m128i h0123; // h0:h1:h2:h3
			__m128i h4567; // h4:h5:h6:h7
		};
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
			struct {
				__m128i _h0145; // h0:h1:h4:h5
				__m128i _h2367; // h2:h3:h6:h7
			};
		};

	private:
		std::array<uint8_t, 64> _buf;
		size_t _len;
	};
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif  // defined(__clang__)

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif  // defined(__GNUC__)

#if defined(_MSC_VER)
#pragma warning( pop )
#endif // defined(_MSC_VER)
