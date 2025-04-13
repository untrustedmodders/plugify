#include "sha256.hpp"

#include "binary_format.h"

using namespace plugify;

namespace {
	PLUGIFY_FORCE_INLINE uint32_t ror(uint32_t n, int k) {
		return (n >> k) | (n << (32 - k));
	}

	PLUGIFY_FORCE_INLINE uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
		return z ^ (x & (y ^ z));
	}

	PLUGIFY_FORCE_INLINE uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
		return (x & y) | (z & (x | y));
	}

	PLUGIFY_FORCE_INLINE uint32_t s0(uint32_t x) {
		return ror(x, 2) ^ ror(x, 13) ^ ror(x, 22);
	}

	PLUGIFY_FORCE_INLINE uint32_t s1(uint32_t x) {
		return ror(x, 6) ^ ror(x, 11) ^ ror(x, 25);
	}

	PLUGIFY_FORCE_INLINE uint32_t r0(uint32_t x) {
		return ror(x, 7) ^ ror(x, 18) ^ (x >> 3);
	}

	PLUGIFY_FORCE_INLINE uint32_t r1(uint32_t x) {
		return ror(x, 17) ^ ror(x, 19) ^ (x >> 10);
	}

	// K Array
	constexpr union {
		uint32_t dw[64];
#if !PLUGIFY_ARCH_ARM
		__m128i x[16];
#endif // !PLUGIFY_ARCH_ARM
	} K = {
			0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
			0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
			0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
			0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
			0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
			0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
			0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
			0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
	};

	constexpr std::array<uint32_t, 8> H{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
}// namespace

#if PLUGIFY_ARCH_ARM
#include "sha256_arm.inl"
#else
#include "sha256_x86.inl"
#endif // PLUGIFY_ARCH_ARM

Sha256::Sha256() {
	clear();
}

void Sha256::clear() {
	_len = 0;

#if !PLUGIFY_ARCH_ARM
	if (has_sha256_acceleration) {
		_state0 = _mm_set_epi32(int(H[0]), int(H[1]), int(H[4]), int(H[5]));
		_state1 = _mm_set_epi32(int(H[2]), int(H[3]), int(H[6]), int(H[7]));
	}
	else
#endif // !PLUGIFY_ARCH_ARM
	{
		_h = H;
	}
}

void Sha256::update(std::span<const uint8_t> in) {
	const auto off = _len % 64;

	_len += in.size();

	// If any bytes are left in the message buffer,
	// fullfill the block first
	if (off) {
		const auto rem = 64 - off;
		if (in.size() < rem) {
			std::memcpy(_buf.data() + off, in.data(), in.size());
			return;
		}
		std::memcpy(_buf.data() + off, in.data(), rem);
		(this->*compress)(_buf);
		in = in.subspan(rem);
	}

	// When we reach here, we have no data left in the message buffer
	while (in.size() >= 64) {
		// No need to copy into the internal message block
		auto block = in.subspan<0, 64>();
		(this->*compress)(block);
		in = in.subspan<64>();
	}

	// Leave the remaining bytes in the message buffer
	std::memcpy(_buf.data(), in.data(), in.size());
}

std::array<uint8_t, 32> Sha256::finalize() {
	auto off = _len % 64;

	// When we reach here, the block is supposed to be unfullfilled.
	// Add the terminating bit
	const uint8_t end_bit = 0x80;
	_buf[off++] = end_bit;

	// Need to set total length in the last 8-byte of the block.
	// If there is no room for the length, process this block first
	if (off > 56) {
		// Fill zeros and compress
		std::memset(_buf.data() + off, 0, 64 - off);
		(this->*compress)(_buf);
		off = 0;
	}

	// Fill zeros before the last 8-byte of the block
	std::memset(_buf.data() + off, 0, 56 - off);

	// Set the length of the message in big-endian
	const auto l = htobe64(_len * 8);
	std::memcpy(_buf.data() + 56, &l, 8);

	// Process the last block
	(this->*compress)(_buf);

	// SHA uses big endian byte ordering
	if (has_sha256_acceleration) {
#if PLUGIFY_ARCH_ARM
#if !PLUGIFY_IS_BIG_ENDIAN
		const uint8x16_t byteswapindex = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

		_state0 = vqtbl1q_u8(_state0, byteswapindex);
		_state1 = vqtbl1q_u8(_state1, byteswapindex);
#endif // !PLUGIFY_IS_BIG_ENDIAN
#else
		// Get the resulting hash value.
		// h0:h1:h4:h5
		// h2:h3:h6:h7
		//      |
		//      V
		// h0:h1:h2:h3
		// h4:h5:h6:h7
		__m128i h0123 = _mm_unpackhi_epi64(_state1, _state0);
		__m128i h4567 = _mm_unpacklo_epi64(_state1, _state0);

#if !PLUGIFY_IS_BIG_ENDIAN
		const __m128i byteswapindex = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

		h0123 = _mm_shuffle_epi8(h0123, byteswapindex);
		h4567 = _mm_shuffle_epi8(h4567, byteswapindex);
#endif // !PLUGIFY_IS_BIG_ENDIAN

		_state0 = h0123;
		_state1 = h4567;
#endif // PLUGIFY_ARCH_ARM
	}
	else
	{
		// Revert all bytes
		for (auto& v : _h)
			v = htobe32(v);
	}

	return _b;
}

void Sha256::compress_base(std::span<const uint8_t, 64> in) {
	std::array<uint32_t, 8> S;
	std::array<uint32_t, 64> W;

	std::memcpy(W.data(), in.data(), 64);

	for (size_t i = 0; i < 16; ++i)
		W[i] = be32toh(W[i]);

	for (size_t i = 16; i < 64; ++i)
		W[i] = r1(W[i - 2]) + W[i - 7] + r0(W[i - 15]) + W[i - 16];

	std::memcpy(S.data(), _h.data(), 32);

	for (size_t i = 0; i < 64; ++i) {
		uint32_t t1 = S[7] + s1(S[4]) + ch(S[4], S[5], S[6]) + K.dw[i] + W[i];
		uint32_t t2 = s0(S[0]) + maj(S[0], S[1], S[2]);
		S[7] = S[6];
		S[6] = S[5];
		S[5] = S[4];
		S[4] = S[3] + t1;
		S[3] = S[2];
		S[2] = S[1];
		S[1] = S[0];
		S[0] = t1 + t2;
	}

	for (size_t i = 0; i < 8; ++i)
		_h[i] += S[i];
}

std::string Sha256::to_string() const {
	return std::format(
		"{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}"
		"{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}"
		"{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}"
		"{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
		_b[0], _b[1], _b[2], _b[3],
		_b[4], _b[5], _b[6], _b[7],
		_b[8], _b[9], _b[10], _b[11],
		_b[12], _b[13], _b[14], _b[15],
		_b[16], _b[17], _b[18], _b[19],
		_b[20], _b[21], _b[22], _b[23],
		_b[24], _b[25], _b[26], _b[27],
		_b[28], _b[29], _b[30], _b[31]
	);
}
