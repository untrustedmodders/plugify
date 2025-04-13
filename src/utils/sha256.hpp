#pragma once

#include <span>
#include <array>
#include <optional>
#include <plugify/macro.hpp>
#include <plugify/compat_format.hpp>

#if PLUGIFY_ARCH_ARM
#include <arm_neon.h>
#else
#if PLUGIFY_COMPILER_GCC && !PLUGIFY_COMPILER_CLANG && !defined(NDEBUG)
#undef __OPTIMIZE__
#endif // !defined(NDEBUG)
#include <emmintrin.h>
#endif // PLUGIFY_USE_ARM

#if PLUGIFY_ARCH_ARM
using uint128_i = uint32x4_t;
#else
using uint128_i = __m128i;
#endif // PLUGIFY_ARCH_ARM

extern bool sha256_simd_available();

namespace plugify {
	PLUGIFY_WARN_PUSH()

#if PLUGIFY_COMPILER_CLANG
	PLUGIFY_WARN_IGNORE("-Wgnu-anonymous-struct")
	PLUGIFY_WARN_IGNORE("-Wnested-anon-types")
#elif PLUGIFY_COMPILER_GCC
	PLUGIFY_WARN_IGNORE("-Wpedantic")
#elif PLUGIFY_COMPILER_MSVC
	PLUGIFY_WARN_IGNORE(4201)
#endif

	class Sha256 {
	public:
		Sha256();

		void clear();
		void update(std::span<const uint8_t> in);
		std::array<uint8_t, 32> finalize();

		std::string to_string() const;

	private:
		void init_base();
		void compress_base(std::span<const uint8_t, 64> in);
		void swap_base();

		void init_simd();
		void compress_simd(std::span<const uint8_t, 64> in);
		void swap_simd();

		union {
			std::array<uint8_t, 32> _b;
			std::array<uint32_t, 8> _h;
			struct {
				uint128_i _state0;
				uint128_i _state1;
			};
		};
		std::array<uint8_t, 64> _buf;
		size_t _len;

		static inline bool has_sha256_acceleration = sha256_simd_available();
		static inline decltype(&Sha256::init_base) init = has_sha256_acceleration ? &Sha256::init_simd : &Sha256::init_base;
		static inline decltype(&Sha256::compress_base) compress = has_sha256_acceleration ? &Sha256::compress_simd : &Sha256::compress_base;
		static inline decltype(&Sha256::swap_base) swap = has_sha256_acceleration ? &Sha256::swap_simd : &Sha256::swap_base;
	};

	PLUGIFY_WARN_POP()
}
