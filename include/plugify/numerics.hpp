#pragma once

#include <plugify/vector.hpp>
#include <plugify/string.hpp>

namespace plg {
	_PLUGIFY_WARN_PUSH()

#if defined(__clang__)
	_PLUGIFY_WARN_IGNORE("-Wgnu-anonymous-struct")
#elif defined(__GNUC__)
	_PLUGIFY_WARN_IGNORE("-Wpedantic")
#elif defined(_MSC_VER)
	_PLUGIFY_WARN_IGNORE(4201)
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
				float data[4][4];
			};
		};

		struct vec {
			[[maybe_unused]] uint8_t padding[sizeof(vector<int>)]{};
		};

		struct str {
			[[maybe_unused]] uint8_t padding[sizeof(string)]{};
		};
	}

	_PLUGIFY_WARN_POP()
} // namespace plg
