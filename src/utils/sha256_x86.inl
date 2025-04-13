#pragma once

#if PLUGIFY_COMPILER_MSVC
#include <intrin.h>
#endif // PLUGIFY_COMPILER_MSVC

#if PLUGIFY_COMPILER_CLANG
#include <cpuid.h>
#include <immintrin.h>
#endif // PLUGIFY_COMPILER_CLANG

#if PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_INTEL
#include <immintrin.h>
#endif// PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_INTEL

bool sha256_simd_available() {
#if PLUGIFY_COMPILER_MSVC
	int32_t regs0[4] = {0,0,0,0}, regs1[4] = {0,0,0,0}, regs7[4] = {0,0,0,0};
	const uint32_t SSSE3_BIT = 1u << 9;  /* Function 1, Bit	9 of ECX */
	const uint32_t SSE41_BIT = 1u << 19; /* Function 1, Bit 19 of ECX */
	const uint32_t SHA_BIT	 = 1u << 29; /* Function 7, Bit 29 of EBX */

	__cpuid(regs0, 0);
	const int32_t highest = regs0[0]; /*EAX*/

	if (highest >= 0x01) {
		__cpuidex(regs1, 1, 0);
	}

	if (highest >= 0x07) {
		__cpuidex(regs7, 7, 0);
	}

	return (regs1[2] /*ECX*/ & SSSE3_BIT) && (regs1[2] /*ECX*/ & SSE41_BIT) && (regs7[1] /*EBX*/ & SHA_BIT);
#elif PLUGIFY_COMPILER_CLANG // && CLANG_AT_LEAST(19, 0, 0)
	// FIXME: Use __builtin_cpu_supports("sha") when compilers support it
	constexpr uint32_t cpuid_sha_ebx = (1 << 29);
	uint32_t eax, ebx, ecx, edx;
	__cpuid_count(7, 0, eax, ebx, ecx, edx);
	const uint32_t cpu_supports_sha = (ebx & cpuid_sha_ebx);
	return __builtin_cpu_supports("ssse3") && __builtin_cpu_supports("sse4.1") && cpu_supports_sha;
#elif PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_CLANG
	/* __builtin_cpu_supports available in GCC 4.8.1 and above */
	return __builtin_cpu_supports("ssse3") && __builtin_cpu_supports("sse4.1") && __builtin_cpu_supports("sha");
#elif PLUGIFY_COMPILER_INTEL
	/* https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_may_i_use_cpu_feature */
	return _may_i_use_cpu_feature(_FEATURE_SSSE3|_FEATURE_SSE4_1|_FEATURE_SHA);
#else
	#warning "sha256_simd_available not implemented on this platform!"
	return false;
#endif
}

void Sha256::compress_simd(std::span<const uint8_t, 64> in) {
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

	__m128i state0 = _state0;// a:b:e:f
	__m128i state1 = _state1;// c:d:g:h
	__m128i tmp;

	/* w0 - w3 */
#define SHA256_ROUNDS_4(cwN, n)                                                                            \
	tmp = _mm_add_epi32(cwN, K.x[n]);					 /* w3+K3 : w2+K2 : w1+K1 : w0+K0 */               \
	state1 = _mm_sha256rnds2_epu32(state1, state0, tmp); /* state1 = a':b':e':f' / state0 = c':d':g':h' */ \
	tmp = _mm_unpackhi_epi64(tmp, tmp);					 /* - : - : w3+K3 : w2+K2 */                       \
	state0 = _mm_sha256rnds2_epu32(state0, state1, tmp); /* state0 = a':b':e':f' / state1 = c':d':g':h' */

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

	// Combine state
	_state0 = _mm_add_epi32(state0, _state0);
	_state1 = _mm_add_epi32(state1, _state1);
}

void Sha256::init_simd() {
	_state0 = _mm_set_epi32(int(H[0]), int(H[1]), int(H[4]), int(H[5]));
	_state1 = _mm_set_epi32(int(H[2]), int(H[3]), int(H[6]), int(H[7]));
}

void Sha256::swap_simd() {
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
}