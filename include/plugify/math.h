#pragma once

#include <array>

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
	extern "C" {
		/**
		* @struct Vector2
		* @brief Represents a 2D vector with x and y components.
		*/
		struct Vector2 {
			union {
				struct {
					float x; ///< x-component of the vector.
					float y; ///< y-component of the vector.
				};
				std::array<float, 2> data{}; ///< Array representation of the vector.
			};

			/**
			* @brief Assignment operator for Vector2.
			* @param other The Vector2 to assign values from.
			* @return Reference to the modified Vector2.
			*/
			Vector2& operator=(const Vector2& other) = default;
		};

		/**
		* @struct Vector3
		* @brief Represents a 3D vector with x, y, and z components.
		*/
		struct Vector3 {
			union {
				struct {
					float x; ///< x-component of the vector.
					float y; ///< y-component of the vector.
					float z; ///< z-component of the vector.
				};
				std::array<float, 3> data{}; ///< Array representation of the vector.
			};

			/**
			* @brief Assignment operator for Vector3.
			* @param other The Vector3 to assign values from.
			* @return Reference to the modified Vector3.
			*/
			Vector3& operator=(const Vector3& other) = default;
		};

		/**
		* @struct Vector4
		* @brief Represents a 4D vector with x, y, z, and w components.
		*/
		struct Vector4 {
			union {
				struct {
					float x; ///< x-component of the vector.
					float y; ///< y-component of the vector.
					float z; ///< z-component of the vector.
					float w; ///< w-component of the vector.
				};
				std::array<float, 4> data{}; ///< Array representation of the vector.
			};

			/**
			* @brief Assignment operator for Vector4.
			* @param other The Vector4 to assign values from.
			* @return Reference to the modified Vector4.
			*/
			Vector4& operator=(const Vector4& other) = default;
		};

		/**
		* @struct Matrix4x4
		* @brief Represents a 4x4 matrix using Vector4 rows.
		*/
		struct Matrix4x4 {
			union {
				struct {
					float m00, m01, m02, m03;
					float m10, m11, m12, m13;
					float m20, m21, m22, m23;
					float m30, m31, m32, m33;
				};
				std::array<float, 16> data{}; ///< Array representation of the matrix.
			};

			/**
			* @brief Assignment operator for Matrix4x4.
			* @param other The Matrix4x4 to assign values from.
			* @return Reference to the modified Matrix4x4.
			*/
			Matrix4x4& operator=(const Matrix4x4& other) = default;
		};
	}
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
