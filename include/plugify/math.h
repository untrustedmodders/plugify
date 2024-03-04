#pragma once

namespace plugify {
	/**
	 * @struct Vector2
	 * @brief Represents a 2D vector with x and y components.
	 */
	struct Vector2 {
		float x{}; /**< x-component of the vector. */
		float y{}; /**< y-component of the vector. */

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
		float x{}; /**< x-component of the vector. */
		float y{}; /**< y-component of the vector. */
		float z{}; /**< z-component of the vector. */

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
		float x{}; /**< x-component of the vector. */
		float y{}; /**< y-component of the vector. */
		float z{}; /**< z-component of the vector. */
		float w{}; /**< w-component of the vector. */

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
		Vector4 rows[4]{}; /**< Rows of the matrix represented by Vector4. */

		/**
		 * @brief Assignment operator for Matrix4x4.
		 * @param other The Matrix4x4 to assign values from.
		 * @return Reference to the modified Matrix4x4.
		 */
		Matrix4x4& operator=(const Matrix4x4& other) = default;
	};

	/**
	 * @struct Matrix3x2
	 * @brief Represents a 3x2 matrix using Vector3 rows.
	 */
	struct Matrix3x2 {
		Vector3 rows[2]{}; /**< Rows of the matrix represented by Vector3. */

		/**
		 * @brief Assignment operator for Matrix3x2.
		 * @param other The Matrix3x2 to assign values from.
		 * @return Reference to the modified Matrix3x2.
		 */
		Matrix3x2& operator=(const Matrix3x2& other) = default;
	};
}