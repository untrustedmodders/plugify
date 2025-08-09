#pragma once

#include <chrono>
#include <iomanip>
#include <cmath>

namespace plugify {
	using namespace std::chrono_literals;

	class DateTime {
	public:
		DateTime() = default;

		/**
		 * @brief Constructs a DateTime object from a duration.
		 * @tparam Rep The representation type of the duration (e.g., int, float).
		 * @tparam Period The period type of the duration (e.g., seconds, milliseconds).
		 * @param duration The duration to initialize the DateTime object.
		 */
		template<typename Rep, typename Period>
		constexpr DateTime(const std::chrono::duration<Rep, Period>& duration) noexcept : _value{std::chrono::duration_cast<std::chrono::microseconds>(duration).count()} {}

		/**
		 * @brief Creates a DateTime object representing seconds.
		 * @tparam T The type of the seconds value (default is float).
		 * @param seconds The number of seconds.
		 * @return A DateTime object representing the given seconds.
		 */
		template<typename T = float>
		constexpr static DateTime Seconds(const T& seconds) noexcept { return {std::chrono::duration<T>(seconds)}; }

		/**
		 * @brief Creates a DateTime object representing milliseconds.
		 * @tparam T The type of the milliseconds value (default is double).
		 * @param milliseconds The number of milliseconds.
		 * @return A DateTime object representing the given milliseconds.
		 */
		template<typename T = double>
		constexpr static DateTime Milliseconds(const T& milliseconds) noexcept { return {std::chrono::duration<T, std::micro>(milliseconds)}; }

		/**
		 * @brief Creates a DateTime object representing microseconds.
		 * @tparam T The type of the microseconds value (default is uint64_t).
		 * @param microseconds The number of microseconds.
		 * @return A DateTime object representing the given microseconds.
		 */
		template<typename T = uint64_t>
		constexpr static DateTime Microseconds(const T& microseconds) noexcept { return {std::chrono::duration<T, std::micro>(microseconds)}; }

		/**
		 * @brief Converts the time duration to seconds.
		 * @tparam T The return type of the value (default is float).
		 * @return The duration as seconds.
		 */
		template<typename T = float>
		constexpr auto AsSeconds() const noexcept { return static_cast<T>(_value.count()) / static_cast<T>(1000000); }

		/**
		 * @brief Converts the time duration to milliseconds.
		 * @tparam T The return type of the value (default is double).
		 * @return The duration as milliseconds.
		 */
		template<typename T = double>
		constexpr auto AsMilliseconds() const noexcept { return static_cast<T>(_value.count()) / static_cast<T>(1000); }

		/**
		 * @brief Converts the time duration to microseconds.
		 * @tparam T The return type of the value (default is uint64_t).
		 * @return The duration as microseconds.
		 */
		template<typename T = uint64_t>
		constexpr auto AsMicroseconds() const noexcept { return static_cast<T>(_value.count()); }

		/**
		 * @brief Gets the current time since a local epoch.
		 * @return A DateTime object representing the current time.
		 */
		static DateTime Now() noexcept {
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - localEpoch);
		}

		/**
		 * @brief Gets the current system time formatted as a string.
		 * @param format The desired time format (default is "%Y-%m-%d %H:%M:%S").
		 * @return A string representation of the current system time.
		 */
		static std::string Get(std::string_view format = "%Y-%m-%d %H:%M:%S") {
			auto now = std::chrono::system_clock::now();
			auto t = std::chrono::system_clock::to_time_t(now);
			std::tm time{};
#if _WIN32
			localtime_s(&time, &t); // Windows-specific
#else
			localtime_r(&t, &time); // POSIX-compliant
#endif
			std::string buffer(80, '\0');
			size_t res = std::strftime(buffer.data(), buffer.size(), format.data(), &time);
			if (!res)
				return "strftime error";
			buffer.resize(res);
			return buffer;
		}

		/**
		 * @brief Converts the DateTime object to a standard chrono duration.
		 * @tparam Rep The representation type for the duration (e.g., int, float).
		 * @tparam Period The period type for the duration (e.g., seconds, milliseconds).
		 * @return The DateTime object as a chrono duration.
		 */
		template<typename Rep, typename Period>
		constexpr explicit operator std::chrono::duration<Rep, Period>() const noexcept {
			return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(_value);
		}

#if __cpp_impl_three_way_comparison
		/**
		 * @brief Compares two DateTime objects using three-way comparison.
		 * @param rhs The right-hand side DateTime object.
		 * @return A comparison result indicating the relationship between the two DateTime objects.
		 */
		constexpr auto operator<=>(const DateTime& rhs) const { return _value <=> rhs._value; }
#endif

		/**
		 * @brief Compares if two DateTime objects are equal.
		 * @param rhs The right-hand side DateTime object.
		 * @return True if equal, false otherwise.
		 */
		constexpr bool operator==(const DateTime& rhs) const noexcept { return _value == rhs._value; }
		/**
		 * @brief Compares if two DateTime objects are not equal.
		 * @param rhs The right-hand side DateTime object.
		 * @return True if not equal, false otherwise.
		 */
		constexpr bool operator!=(const DateTime& rhs) const noexcept { return _value != rhs._value; }
		/**
		 * @brief Compares if one DateTime object is less than another.
		 * @param rhs The right-hand side DateTime object.
		 * @return True if less than, false otherwise.
		 */
		constexpr bool operator<(const DateTime& rhs) const noexcept { return _value < rhs._value; }
		/**
		 * @brief Compares if one DateTime object is less than or equal to another.
		 * @param rhs The right-hand side DateTime object.
		 * @return True if less than or equal, false otherwise.
		 */
		constexpr bool operator<=(const DateTime& rhs) const noexcept { return _value <= rhs._value; }
		/**
		 * @brief Compares if one DateTime object is greater than another.
		 * @param rhs The right-hand side DateTime object.
		 * @return True if greater than, false otherwise.
		 */
		constexpr bool operator>(const DateTime& rhs) const noexcept { return _value > rhs._value; }
		/**
		 * @brief Compares if one DateTime object is greater than or equal to another.
		 * @param rhs The right-hand side DateTime object.
		 * @return True if greater than or equal, false otherwise.
		 */
		constexpr bool operator>=(const DateTime& rhs) const noexcept { return _value >= rhs._value; }
		/**
		 * @brief Negates the DateTime value.
		 * @return A negated DateTime object.
		 */
		constexpr DateTime operator-() const noexcept { return { -_value}; }

		/**
		 * @brief Adds two DateTime objects.
		 * @param lhs The left-hand side DateTime object.
		 * @param rhs The right-hand side DateTime object.
		 * @return A DateTime object representing the sum of lhs and rhs.
		 */
		constexpr friend DateTime operator+(const DateTime& lhs, const DateTime& rhs) noexcept { return lhs._value + rhs._value; }
		/**
		 * @brief Subtracts one DateTime object from another.
		 * @param lhs The left-hand side DateTime object.
		 * @param rhs The right-hand side DateTime object.
		 * @return A DateTime object representing the difference between lhs and rhs.
		 */
		constexpr friend DateTime operator-(const DateTime& lhs, const DateTime& rhs) noexcept { return lhs._value - rhs._value; }
		/**
		 * @brief Multiplies a DateTime object by a floating-point value.
		 * @param lhs The DateTime object.
		 * @param rhs The multiplier value.
		 * @return A DateTime object scaled by rhs.
		 */
		constexpr friend DateTime operator*(const DateTime& lhs, float rhs) noexcept { return lhs._value * rhs; }
		/**
		 * @brief Multiplies a DateTime object by an integer value.
		 * @param lhs The DateTime object.
		 * @param rhs The multiplier value.
		 * @return A DateTime object scaled by rhs.
		 */
		constexpr friend DateTime operator*(const DateTime& lhs, int64_t rhs) noexcept { return lhs._value * rhs; }
		/**
		 * @brief Multiplies a floating-point value by a DateTime object.
		 * @param lhs The multiplier value.
		 * @param rhs The DateTime object.
		 * @return A DateTime object scaled by lhs.
		 */
		constexpr friend DateTime operator*(float lhs, const DateTime& rhs) noexcept { return rhs * lhs; }
		/**
		 * @brief Multiplies an integer value by a DateTime object.
		 * @param lhs The multiplier value.
		 * @param rhs The DateTime object.
		 * @return A DateTime object scaled by lhs.
		 */
		constexpr friend DateTime operator*(int64_t lhs, const DateTime& rhs) noexcept { return rhs * lhs; }
		/**
		 * @brief Divides a DateTime object by a floating-point value.
		 * @param lhs The DateTime object.
		 * @param rhs The divisor value.
		 * @return A DateTime object scaled by the inverse of rhs.
		 */
		constexpr friend DateTime operator/(const DateTime& lhs, float rhs) noexcept { return lhs._value / rhs; }
		/**
		 * @brief Divides a DateTime object by an integer value.
		 * @param lhs The DateTime object.
		 * @param rhs The divisor value.
		 * @return A DateTime object scaled by the inverse of rhs.
		 */
		constexpr friend DateTime operator/(const DateTime& lhs, int64_t rhs) noexcept { return lhs._value / rhs; }
		/**
		 * @brief Divides one DateTime object by another.
		 * @param lhs The left-hand side DateTime object.
		 * @param rhs The right-hand side DateTime object.
		 * @return The ratio of lhs to rhs as a double.
		 */
		constexpr friend double operator/(const DateTime& lhs, const DateTime& rhs) noexcept { return static_cast<double>(lhs._value.count()) / static_cast<double>(rhs._value.count()); }

		/**
		 * @brief Computes the modulo (remainder) of one DateTime object divided by another.
		 * @tparam Period The time unit for the modulo operation (default is seconds).
		 * @param lhs The left-hand side DateTime object.
		 * @param rhs The right-hand side DateTime object.
		 * @return The modulo result as a double in the specified Period.
		 */
		template<typename Period = std::ratio<1, 1>>
		constexpr friend double operator%(const DateTime& lhs, const DateTime& rhs) {
			return std::modf(std::chrono::duration_cast<std::chrono::duration<double, Period>>(lhs._value), std::chrono::duration_cast<std::chrono::duration<double, Period>>(rhs._value));
		}

		/**
		 * @brief Adds another DateTime object to this one.
		 * @param rhs The DateTime object to add.
		 * @return A reference to the modified DateTime object.
		 */
		constexpr DateTime& operator+=(const DateTime& rhs) noexcept { return *this = *this + rhs; }
		/**
		 * @brief Subtracts another DateTime object from this one.
		 * @param rhs The DateTime object to subtract.
		 * @return A reference to the modified DateTime object.
		 */
		constexpr DateTime& operator-=(const DateTime& rhs) noexcept { return *this = *this - rhs; }
		/**
		 * @brief Multiplies this DateTime object by a floating-point value.
		 * @param rhs The multiplier value.
		 * @return A reference to the modified DateTime object.
		 */
		constexpr DateTime& operator*=(float rhs) noexcept { return *this = *this * rhs; }
		/**
		 * @brief Multiplies this DateTime object by an integer value.
		 * @param rhs The multiplier value.
		 * @return A reference to the modified DateTime object.
		 */
		constexpr DateTime& operator*=(int64_t rhs) noexcept{ return *this = *this * rhs; }
		/**
		 * @brief Divides this DateTime object by a floating-point value.
		 * @param rhs The divisor value.
		 * @return A reference to the modified DateTime object.
		 */
		constexpr DateTime& operator/=(float rhs) noexcept { return *this = *this / rhs; }
		/**
		 * @brief Divides this DateTime object by an integer value.
		 * @param rhs The divisor value.
		 * @return A reference to the modified DateTime object.
		 */
		constexpr DateTime& operator/=(int64_t rhs) noexcept { return *this = *this / rhs; }

	private:
		std::chrono::microseconds _value{}; //!< Internal representation of the time value in microseconds.

		static inline const std::chrono::high_resolution_clock::time_point localEpoch = std::chrono::high_resolution_clock::now();
	};
} // namespace plugify
