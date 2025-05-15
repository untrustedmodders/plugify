#pragma once

#if PLUGIFY_PLATFORM_ANDROID
#include <cpu-features.h>
#elif PLUGIFY_PLATFORM_WINDOWS
#include <processthreadsapi.h>
#elif PLUGIFY_PLATFORM_APPLE
#include <sys/sysctl.h>
#elif PLUGIFY_PLATFORM_LINUX
#include <sys/auxv.h>z
#include <asm/hwcap.h>
#endif

#include <arm_neon.h>

bool sha256_simd_available() {
#if PLUGIFY_PLATFORM_ANDROID && PLUGIFY_ARCH_BITS == 64
	if (((android_getCpuFamily() & ANDROID_CPU_FAMILY_ARM64) != 0) &&
		((android_getCpuFeatures() & ANDROID_CPU_ARM64_FEATURE_SHA2) != 0))
		return true;
#elif PLUGIFY_PLATFORM_ANDROID && PLUGIFY_ARCH_BITS == 32
	if (((android_getCpuFamily() & ANDROID_CPU_FAMILY_ARM) != 0) &&
		((android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_SHA2) != 0))
		return true;
#elif PLUGIFY_PLATFORM_LINUX && PLUGIFY_ARCH_BITS == 64
	if ((getauxval(AT_HWCAP) & HWCAP_SHA2) != 0)
		return true;
#elif PLUGIFY_PLATFORM_LINUX && PLUGIFY_ARCH_BITS == 32
	if ((getauxval(AT_HWCAP2) & HWCAP2_SHA2) != 0)
		return true;
#elif PLUGIFY_PLATFORM_APPLE && PLUGIFY_ARCH_BITS == 64
	int64_t supported = 0;
	size_t sz = sizeof(supported);
	if (sysctlbyname("hw.optional.arm.FEAT_SHA256", &supported, &sz, nullptr, 0) == 0)
		return supported != 0;
#elif PLUGIFY_PLATFORM_WINDOWS && PLUGIFY_ARCH_BITS == 64
	if (IsProcessorFeaturePresent(PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE) != 0)
		return true;
#endif
	return false;
}

void Sha256::compress_simd(std::span<const uint8_t, 64> in) {
	// Load initial values.
    uint32x4_t state0 = _state0; // h0:h1:h2:h3
    uint32x4_t state1 = _state1; // h4:h5:h6:h7

    // Load input block.
    uint32x4_t msg0 = vld1q_u32(reinterpret_cast<const uint32_t *>(&in[0]));
    uint32x4_t msg1 = vld1q_u32(reinterpret_cast<const uint32_t *>(&in[16]));
    uint32x4_t msg2 = vld1q_u32(reinterpret_cast<const uint32_t *>(&in[32]));
    uint32x4_t msg3 = vld1q_u32(reinterpret_cast<const uint32_t *>(&in[48]));

    // Swap bytes on little endian Arm64.
    msg0 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(msg0)));
    msg1 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(msg1)));
    msg2 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(msg2)));
    msg3 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(msg3)));

	// Round computation.
	uint32x4_t msg, tmp;
#define RND4(M, n)                                \
	msg = vaddq_u32(M, K.x[n]);                   \
	tmp = vsha256hq_u32(state0, state1, msg);     \
	state1 = vsha256h2q_u32(state1, state0, msg); \
	state0 = tmp;

	// Message schedule computation
#define MSG4(X0, X1, X2, X3) \
	X0 = vsha256su1q_u32(vsha256su0q_u32(X0, X1), X2, X3)

    // Rounds 0-3
	RND4(msg0, 0);
    MSG4(msg0, msg1, msg2, msg3);

    // Rounds 4-7
	RND4(msg1, 1);
    MSG4(msg1, msg2, msg3, msg0);

    // Rounds 8-11
	RND4(msg2, 2);
    MSG4(msg2, msg3, msg0, msg1);

    // Rounds 12-15
	RND4(msg3, 3);
    MSG4(msg3, msg0, msg1, msg2);

    // Rounds 16-19
	RND4(msg0, 4);
    MSG4(msg0, msg1, msg2, msg3);

    // Rounds 20-23
	RND4(msg1, 5);
    MSG4(msg1, msg2, msg3, msg0);

    // Rounds 24-27
	RND4(msg2, 6);
    MSG4(msg2, msg3, msg0, msg1);

    // Rounds 28-31
	RND4(msg3, 7);
    MSG4(msg3, msg0, msg1, msg2);

    // Rounds 32-35
	RND4(msg0, 8);
    MSG4(msg0, msg1, msg2, msg3);

    // Rounds 36-39
	RND4(msg1, 9);
    MSG4(msg1, msg2, msg3, msg0);

    // Rounds 40-43
	RND4(msg2, 10);
    MSG4(msg2, msg3, msg0, msg1);

    // Rounds 44-47
	RND4(msg3, 11);
    MSG4(msg3, msg0, msg1, msg2);

    // Rounds 48-51
	RND4(msg0, 12);

    // Rounds 52-55
	RND4(msg1, 13);

    // Rounds 56-59
	RND4(msg2, 14);

    // Rounds 60-63
	RND4(msg3, 15);

    // Add back to state
    _state0 = vaddq_u32(state0, _state0);
    _state1 = vaddq_u32(state1, _state1);
}

void Sha256::init_simd() {
	_state0 = { H[0], H[1], H[2], H[3] };
	_state1 = { H[4], H[5], H[6], H[7] };
}

void Sha256::swap_simd() {
#if !PLUGIFY_IS_BIG_ENDIAN
	const uint8x16_t byteswapindex = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

	_state0 = vqtbl1q_u8(_state0, byteswapindex);
	_state1 = vqtbl1q_u8(_state1, byteswapindex);
#endif // !PLUGIFY_IS_BIG_ENDIAN
}