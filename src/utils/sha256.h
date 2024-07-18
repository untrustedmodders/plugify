#pragma once

#include <emmintrin.h>

namespace plugify {
	class Sha256 {
	public:
		Sha256();
		//static constexpr size_t blocklen = 64;
		//static constexpr size_t digestlen = 32;

		void clear();
		void update(std::span<const uint8_t>);

		std::array<uint8_t, 32> digest();

		struct Hash {
			__m128i h0145; // h0:h1:h4:h5
			__m128i h2367; // h2:h3:h6:h7
		};

		static std::string ToString(const std::array<uint8_t, 32>& digest);

	private:
		void process(std::span<const uint8_t, 64> in);

		// Intermediate hash
		union {
			std::array<uint32_t, 8> _h;
			Hash _hash;
		};

		std::array<uint8_t, 64> _buf;
		uint64_t _len;
	};
}