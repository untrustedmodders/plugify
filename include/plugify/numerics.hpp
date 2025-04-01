#pragma once

#include "vector.hpp"
#include "string.hpp"

namespace plg {
	PLUGIFY_WARN_PUSH()

#if PLUGIFY_COMPILER_CLANG
	PLUGIFY_WARN_IGNORE("-Wgnu-anonymous-struct")
	PLUGIFY_WARN_IGNORE("-Wnested-anon-types")
#elif PLUGIFY_COMPILER_GCC
	PLUGIFY_WARN_IGNORE("-Wpedantic")
#elif PLUGIFY_COMPILER_MSVC
	PLUGIFY_WARN_IGNORE(4201)
#endif

	extern "C" {
		struct vec2 {
			union {
				struct {
					float x;
					float y;
				};
				float data[2];
			};
		};

		struct vec3 {
			union {
				struct {
					float x;
					float y;
					float z;
				};
				float data[3];
			};
		};

		struct vec4 {
			union {
				struct {
					float x;
					float y;
					float z;
					float w;
				};
				float data[4];
			};
		};

		struct mat4x4 {
			union {
				struct {
					float m00, m01, m02, m03;
					float m10, m11, m12, m13;
					float m20, m21, m22, m23;
					float m30, m31, m32, m33;
				};
				float m[4][4];
				float data[16];
			};
		};
	}

	PLUGIFY_WARN_POP()

	constexpr float epsilon = 1e-5f;

	// TODO: Replace by std::fabs in C++23
	constexpr float fabs(float x) noexcept {
		return x < 0 ? -x : x;
	}

	constexpr bool operator==(vec2 lhs, vec2 rhs) {
		return fabs(lhs.x - rhs.x) < epsilon && fabs(lhs.y - rhs.y) < epsilon;
	}

	constexpr bool operator==(vec3 lhs, vec3 rhs) {
		return fabs(lhs.x - rhs.x) < epsilon && fabs(lhs.y - rhs.y) < epsilon && fabs(lhs.z - rhs.z) < epsilon;
	}

	constexpr bool operator==(vec4 lhs, vec4 rhs) {
		return fabs(lhs.x - rhs.x) < epsilon && fabs(lhs.y - rhs.y) < epsilon && fabs(lhs.z - rhs.z) < epsilon && fabs(lhs.w - rhs.w) < epsilon;
	}

	constexpr bool operator==(const mat4x4& lhs, const mat4x4& rhs) {
		for (int i = 0; i < 16; ++i) {
			if (fabs(lhs.data[i] - rhs.data[i]) > epsilon)
				return false;
		}
		return true;
	}
} // namespace plg
