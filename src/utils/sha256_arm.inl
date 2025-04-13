#if PLUGIFY_ARCH_ARM

#include <sys/auxv.h>
#include <arm_neon.h>

bool sha256_simd_available() {
	// ARM Cryptography Extensions (SHA256 feature)
	// ID_AA64ISAR0_EL1 feature register, bits 15-12 (SHA2)

	// user space detect through getauxval()
	return (getauxval(AT_HWCAP) & HWCAP_SHA2) ? true : false;
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

    // Rounds 0-3
    uint32x4_t msg = vaddq_u32(msg0, vld1q_u32(&K.dw[4*0]));
    uint32x4_t tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg0 = vsha256su1q_u32(vsha256su0q_u32(msg0, msg1), msg2, msg3);

    // Rounds 4-7
    msg = vaddq_u32(msg1, vld1q_u32(&K.dw[4*1]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg1 = vsha256su1q_u32(vsha256su0q_u32(msg1, msg2), msg3, msg0);

    // Rounds 8-11
    msg = vaddq_u32(msg2, vld1q_u32(&K.dw[4*2]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg2 = vsha256su1q_u32(vsha256su0q_u32(msg2, msg3), msg0, msg1);

    // Rounds 12-15
    msg = vaddq_u32(msg3, vld1q_u32(&K.dw[4*3]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg3 = vsha256su1q_u32(vsha256su0q_u32(msg3, msg0), msg1, msg2);

    // Rounds 16-19
    msg = vaddq_u32(msg0, vld1q_u32(&K.dw[4*4]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg0 = vsha256su1q_u32(vsha256su0q_u32(msg0, msg1), msg2, msg3);

    // Rounds 20-23
    msg = vaddq_u32(msg1, vld1q_u32(&K.dw[4*5]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg1 = vsha256su1q_u32(vsha256su0q_u32(msg1, msg2), msg3, msg0);

    // Rounds 24-27
    msg = vaddq_u32(msg2, vld1q_u32(&K.dw[4*6]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg2 = vsha256su1q_u32(vsha256su0q_u32(msg2, msg3), msg0, msg1);

    // Rounds 28-31
    msg = vaddq_u32(msg3, vld1q_u32(&K.dw[4*7]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg3 = vsha256su1q_u32(vsha256su0q_u32(msg3, msg0), msg1, msg2);

    // Rounds 32-35
    msg = vaddq_u32(msg0, vld1q_u32(&K.dw[4*8]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg0 = vsha256su1q_u32(vsha256su0q_u32(msg0, msg1), msg2, msg3);

    // Rounds 36-39
    msg = vaddq_u32(msg1, vld1q_u32(&K.dw[4*9]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg1 = vsha256su1q_u32(vsha256su0q_u32(msg1, msg2), msg3, msg0);

    // Rounds 40-43
    msg = vaddq_u32(msg2, vld1q_u32(&K.dw[4*10]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg2 = vsha256su1q_u32(vsha256su0q_u32(msg2, msg3), msg0, msg1);

    // Rounds 44-47
    msg = vaddq_u32(msg3, vld1q_u32(&K.dw[4*11]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;
    msg3 = vsha256su1q_u32(vsha256su0q_u32(msg3, msg0), msg1, msg2);

    // Rounds 48-51
    msg = vaddq_u32(msg0, vld1q_u32(&K.dw[4*12]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;

    // Rounds 52-55
    msg = vaddq_u32(msg1, vld1q_u32(&K.dw[4*13]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;

    // Rounds 56-59
    msg = vaddq_u32(msg2, vld1q_u32(&K.dw[4*14]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;

    // Rounds 60-63
    msg = vaddq_u32(msg3, vld1q_u32(&K.dw[4*15]));
    tmp = vsha256hq_u32(state0, state1, msg);
    state1 = vsha256h2q_u32(state1, state0, msg);
    state0 = tmp;

    // Add back to state
    _state0 = vaddq_u32(state0, _state0);
    _state1 = vaddq_u32(state1, _state1);
}

#endif // PLUGIFY_ARCH_ARM
