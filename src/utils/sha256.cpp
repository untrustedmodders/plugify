#include "sha256.h"
#include "binary_format.h"

#if PLUGIFY_COMPILER_MSVC
#include <intrin.h>
#endif // PLUGIFY_COMPILER_MSVC

#if PLUGIFY_COMPILER_CLANG
#include <cpuid.h>
#include <immintrin.h>
#endif  // PLUGIFY_COMPILER_CLANG

#if PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_INTEL
#include <immintrin.h>
#endif// PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_INTEL

/*
 * Detect if the processor supports SHA-256 acceleration. We only check for
 * the three ISAs we need - SSSE3, SSE4.1 and SHA. We don't check for OS
 * support or XSAVE because that's been enabled since Windows 2000.
 */
bool DetectSHA256Acceleration() {
#if PLUGIFY_COMPILER_MSVC
	int32_t regs0[4] = {0,0,0,0}, regs1[4] = {0,0,0,0}, regs7[4] = {0,0,0,0};
	const uint32_t SSSE3_BIT = 1u <<  9; /* Function 1, Bit  9 of ECX */
	const uint32_t SSE41_BIT = 1u << 19; /* Function 1, Bit 19 of ECX */
	const uint32_t SHA_BIT   = 1u << 29; /* Function 7, Bit 29 of EBX */

	__cpuid(regs0, 0);
	const int32_t highest = regs0[0]; /*EAX*/

	if (highest >= 0x01) {
		__cpuidex(regs1, 1, 0);
	}

	if (highest >= 0x07) {
		__cpuidex(regs7, 7, 0);
	}

	return (regs1[2] /*ECX*/ & SSSE3_BIT) && (regs1[2] /*ECX*/ & SSE41_BIT) && (regs7[1] /*EBX*/ & SHA_BIT);
#elif PLUGIFY_COMPILER_CLANG
	// FIXME: Use __builtin_cpu_supports("sha") when compilers support it
	constexpr uint32_t cpuid_sha_ebx = (1 << 29);
	uint32_t eax, ebx, ecx, edx;
	__cpuid_count(7, 0, eax, ebx, ecx, edx);
	const uint32_t cpu_supports_sha = (ebx & cpuid_sha_ebx);
	return __builtin_cpu_supports("ssse3") && __builtin_cpu_supports("sse4.1") && cpu_supports_sha;
#elif PLUGIFY_COMPILER_GCC
	/* __builtin_cpu_supports available in GCC 4.8.1 and above */
	return __builtin_cpu_supports("ssse3") && __builtin_cpu_supports("sse4.1") && __builtin_cpu_supports("sha");
#elif PLUGIFY_COMPILER_INTEL
	/* https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_may_i_use_cpu_feature */
	return _may_i_use_cpu_feature(_FEATURE_SSSE3|_FEATURE_SSE4_1|_FEATURE_SHA);
#else
	return false;
#endif
}

using namespace plugify;

namespace {
	uint32_t ror(uint32_t n, int k) {
		return (n >> k) | (n << (32 - k));
	}

	uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
		return z ^ (x & (y ^ z));
	}

	uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
		return (x & y) | (z & (x | y));
	}

	uint32_t s0(uint32_t x) {
		return ror(x, 2) ^ ror(x, 13) ^ ror(x, 22);
	}

	uint32_t s1(uint32_t x) {
		return ror(x, 6) ^ ror(x, 11) ^ ror(x, 25);
	}

	uint32_t r0(uint32_t x) {
		return ror(x, 7) ^ ror(x, 18) ^ (x >> 3);
	}

	uint32_t r1(uint32_t x) {
		return ror(x, 17) ^ ror(x, 19) ^ (x >> 10);
	}

	// K Array
	constexpr union {
		uint32_t dw[64];
		__m128i x[16];
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

	void transform_impl_base(Sha256& sha, std::span<const uint8_t, 64> in) {
		std::array<uint32_t, 8> S;
		std::array<uint32_t, 64> W;

		memcpy(W.data(), in.data(), 64);

		for (size_t i = 0; i < 16; ++i)
			W[i] = be32toh(W[i]);

		for (size_t i = 16; i < 64; ++i)
			W[i] = r1(W[i - 2]) + W[i - 7] + r0(W[i - 15]) + W[i - 16];

		memcpy(S.data(), sha._h.data(), 32);

		for (size_t i = 0; i < 64; i++) {
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
			sha._h[i] += S[i];
	}

	/**
	 * Based on: https://github.com/stong/bruteforce/tree/master
	 */
	void transform_impl_sha(Sha256& sha, std::span<const uint8_t, 64> in) {
		// Cyclic W array
		// We keep the W array content cyclically in 4 variables
		// Initially:
		// cw0 = w3 : w2 : w1 : w0
		// cw1 = w7 : w6 : w5 : w4
		// cw2 = w11 : w10 : w9 : w8
		// cw3 = w15 : w14 : w13 : w12
		const __m128i byteswapindex = _mm_set_epi8(12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3);
		const __m128i* msgx = reinterpret_cast<const __m128i*>(in.data());
		__m128i cw0 = _mm_shuffle_epi8(_mm_loadu_si128(msgx), byteswapindex);
		__m128i cw1 = _mm_shuffle_epi8(_mm_loadu_si128(msgx + 1), byteswapindex);
		__m128i cw2 = _mm_shuffle_epi8(_mm_loadu_si128(msgx + 2), byteswapindex);
		__m128i cw3 = _mm_shuffle_epi8(_mm_loadu_si128(msgx + 3), byteswapindex);

// Advance W array cycle
// Inputs:
//  CW0 = w[t-13] : w[t-14] : w[t-15] : w[t-16]
//  CW1 = w[t-9] : w[t-10] : w[t-11] : w[t-12]
//  CW2 = w[t-5] : w[t-6] : w[t-7] : w[t-8]
//  CW3 = w[t-1] : w[t-2] : w[t-3] : w[t-4]
// Outputs:
//  CW1 = w[t-9] : w[t-10] : w[t-11] : w[t-12]
//  CW2 = w[t-5] : w[t-6] : w[t-7] : w[t-8]
//  CW3 = w[t-1] : w[t-2] : w[t-3] : w[t-4]
//  CW0 = w[t+3] : w[t+2] : w[t+1] : w[t]
#define CYCLE_W(CW0, CW1, CW2, CW3)                                                              \
	CW0 = _mm_sha256msg1_epu32(CW0, CW1);                                                        \
	CW0 = _mm_add_epi32(CW0, _mm_alignr_epi8(CW3, CW2, 4)); /* add w[t-4]:w[t-5]:w[t-6]:w[t-7]*/ \
	CW0 = _mm_sha256msg2_epu32(CW0, CW3);

		__m128i state1 = sha._h0145;// a:b:e:f
		__m128i state2 = sha._h2367;// c:d:g:h
		__m128i tmp;

		/* w0 - w3 */
#define SHA256_ROUNDS_4(cwN, n)                                                                            \
	tmp = _mm_add_epi32(cwN, K.x[n]);					 /* w3+K3 : w2+K2 : w1+K1 : w0+K0 */               \
	state2 = _mm_sha256rnds2_epu32(state2, state1, tmp); /* state2 = a':b':e':f' / state1 = c':d':g':h' */ \
	tmp = _mm_unpackhi_epi64(tmp, tmp);					 /* - : - : w3+K3 : w2+K2 */                       \
	state1 = _mm_sha256rnds2_epu32(state1, state2, tmp); /* state1 = a':b':e':f' / state2 = c':d':g':h' */

		/* w0 - w3 */
		SHA256_ROUNDS_4(cw0, 0);
		/* w4 - w7 */
		SHA256_ROUNDS_4(cw1, 1);
		/* w8 - w11 */
		SHA256_ROUNDS_4(cw2, 2);
		/* w12 - w15 */
		SHA256_ROUNDS_4(cw3, 3);
		/* w16 - w19 */
		CYCLE_W(cw0, cw1, cw2, cw3); /* cw0 = w19 : w18 : w17 : w16 */
		SHA256_ROUNDS_4(cw0, 4);
		/* w20 - w23 */
		CYCLE_W(cw1, cw2, cw3, cw0); /* cw1 = w23 : w22 : w21 : w20 */
		SHA256_ROUNDS_4(cw1, 5);
		/* w24 - w27 */
		CYCLE_W(cw2, cw3, cw0, cw1); /* cw2 = w27 : w26 : w25 : w24 */
		SHA256_ROUNDS_4(cw2, 6);
		/* w28 - w31 */
		CYCLE_W(cw3, cw0, cw1, cw2); /* cw3 = w31 : w30 : w29 : w28 */
		SHA256_ROUNDS_4(cw3, 7);
		/* w32 - w35 */
		CYCLE_W(cw0, cw1, cw2, cw3); /* cw0 = w35 : w34 : w33 : w32 */
		SHA256_ROUNDS_4(cw0, 8);
		/* w36 - w39 */
		CYCLE_W(cw1, cw2, cw3, cw0); /* cw1 = w39 : w38 : w37 : w36 */
		SHA256_ROUNDS_4(cw1, 9);
		/* w40 - w43 */
		CYCLE_W(cw2, cw3, cw0, cw1); /* cw2 = w43 : w42 : w41 : w40 */
		SHA256_ROUNDS_4(cw2, 10);
		/* w44 - w47 */
		CYCLE_W(cw3, cw0, cw1, cw2); /* cw3 = w47 : w46 : w45 : w44 */
		SHA256_ROUNDS_4(cw3, 11);
		/* w48 - w51 */
		CYCLE_W(cw0, cw1, cw2, cw3); /* cw0 = w51 : w50 : w49 : w48 */
		SHA256_ROUNDS_4(cw0, 12);
		/* w52 - w55 */
		CYCLE_W(cw1, cw2, cw3, cw0); /* cw1 = w55 : w54 : w53 : w52 */
		SHA256_ROUNDS_4(cw1, 13);
		/* w56 - w59 */
		CYCLE_W(cw2, cw3, cw0, cw1); /* cw2 = w59 : w58 : w57 : w56 */
		SHA256_ROUNDS_4(cw2, 14);
		/* w60 - w63 */
		CYCLE_W(cw3, cw0, cw1, cw2); /* cw3 = w63 : w62 : w61 : w60 */
		SHA256_ROUNDS_4(cw3, 15);

		// Add to the intermediate hash
		sha._h0145 = _mm_add_epi32(state1, sha._h0145);
		sha._h2367 = _mm_add_epi32(state2, sha._h2367);
	}

}// namespace

static bool has_sha256_acceleration = DetectSHA256Acceleration();
static decltype(&transform_impl_base) transform_impl = has_sha256_acceleration ? transform_impl_sha : transform_impl_base;
#define transform(i) transform_impl(*this, i);

Sha256::Sha256() {
	clear();
}

void Sha256::clear() {
	_len = 0;

	if (has_sha256_acceleration) {
		_h0145 = _mm_set_epi32(int(H[0]), int(H[1]), int(H[4]), int(H[5]));
		_h2367 = _mm_set_epi32(int(H[2]), int(H[3]), int(H[6]), int(H[7]));
	} else {
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
		transform(_buf);
		in = in.subspan(rem);
	}

	// When we reach here, we have no data left in the message buffer
	while (in.size() >= 64) {
		// No need to copy into the internal message block
		auto block = in.subspan<0, 64>();
		transform(block);
		in = in.subspan<64>();
	}

	// Leave the remaining bytes in the message buffer
	std::memcpy(_buf.data(), in.data(), in.size());
}

Digest Sha256::digest() {
	auto off = _len % 64;

	// When we reach here, the block is supposed to be unfullfilled.
	// Add the terminating bit
	_buf[off++] = 0x80;

	// Need to set total length in the last 8-byte of the block.
	// If there is no room for the length, process this block first
	if (off > 56) {
		// Fill zeros and compress
		std::memset(_buf.data() + off, 0, 64 - off);
		transform(_buf);
		off = 0;
	}

	// Fill zeros before the last 8-byte of the block
	std::memset(_buf.data() + off, 0, 56 - off);

	// Set the length of the message in big-endian
	const auto l = htobe64(_len * 8);
	std::memcpy(_buf.data() + 56, &l, 8);

	// Process the last block
	transform(_buf);

	Digest digest;

	// SHA uses big endian byte ordering
	if (has_sha256_acceleration) {
		// Get the resulting hash value.
		// h0:h1:h4:h5
		// h2:h3:h6:h7
		//      |
		//      V
		// h0:h1:h2:h3
		// h4:h5:h6:h7
		digest.h0123 = _mm_unpackhi_epi64(_h2367, _h0145);
		digest.h4567 = _mm_unpacklo_epi64(_h2367, _h0145);

#if !PLUGIFY_IS_BIG_ENDIAN
		// Swap the byte order
		const __m128i byteswapindex = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

		digest.h0123 = _mm_shuffle_epi8(digest.h0123, byteswapindex);
		digest.h4567 = _mm_shuffle_epi8(digest.h4567, byteswapindex);
#endif
	} else {
		// Revert all bytes
		for (auto& v : _h)
			v = htobe32(v);

		std::memcpy(digest.h.data(), _h.data(), 32);
	}

	return digest;
}

std::string Sha256::ToString(const Digest& digest) {
	std::stringstream s;
	s << std::setfill('0') << std::hex;
	
	for(uint8_t i = 0 ; i < 32 ; i++) {
		s << std::setw(2) << static_cast<uint32_t>(digest.h[i]);
	}

	return s.str();
}