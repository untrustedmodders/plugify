#pragma once

#include <optional>
#include <string>
#include <vector>

#include "plg/enum.hpp"
#include "plg/expected.hpp"
#include "plg/inplace_vector.hpp"
#include "plg/source_location.hpp"
#include "plg/format.hpp"
#include "plg/path.hpp"
#include "plg/flat.hpp"
#include "plg/version.hpp"

namespace plugify {
	/**
	 * @typedef Version
	 * @brief Represents a version of a extension.
	 *
	 * This type is an alias for plg::version, which encapsulates the versioning
	 * information for extensions in the Plugify ecosystem.
	 */
	using Version = plg::version<>;

	/**
	 * @typedef Constraint
	 * @brief A set of version ranges a version is checked against.
	 *
	 * Built from the constraint expressions a manifest writes, where comparisons
	 * separated by spaces all have to hold and groups separated by '||' are
	 * alternatives. An empty set accepts every version.
	 *
	 * @see Dependency::GetConstraints, Conflict::GetConstraints
	 */
	using Constraint = plg::range_set<>;

	/**
	 * @typedef Location
	 * @brief The place in the source a call came from.
	 *
	 * An alias for plg::source_location, used by the logging and profiling
	 * interfaces so a message can name the file, line and function it originated
	 * in without the caller passing them by hand.
	 *
	 * @see ILogger::Log, IProfiler::BeginZone
	 */
	using Location = plg::source_location;

	/**
	 * @typedef UniqueId
	 * @brief Represents a unique identifier for everything.
	 *
	 * A strongly typed counter: it converts to its underlying integer, compares,
	 * and increments like one, but will not be mixed up with an unrelated number.
	 * Identifiers are handed out by the manager as extensions are discovered.
	 *
	 * @see Extension::GetId
	 */
	struct UniqueId {
		/**
		 * @typedef Value
		 * @brief The underlying integer type of an identifier.
		 */
		using Value = std::ptrdiff_t;

		/**
		 * @brief Construct the zero identifier.
		 */
		constexpr UniqueId() noexcept = default;

		/**
		 * @brief Construct an identifier from its underlying value.
		 * @param v The underlying value.
		 */
		constexpr explicit UniqueId(Value v) noexcept : value(v) {}

		/**
		 * @brief Destructor.
		 */
		constexpr ~UniqueId() = default;

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of UniqueId.
		 */
		constexpr UniqueId(const UniqueId& other) = default;

		/**
		 * @brief Move constructor.
		 * @param other Another instance of UniqueId.
		 */
		constexpr UniqueId(UniqueId&& other) noexcept = default;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of UniqueId.
		 * @return A reference to this instance after copying.
		 */
		constexpr UniqueId& operator=(const UniqueId& other) = default;

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of UniqueId.
		 * @return A reference to this instance after moving.
		 */
		constexpr UniqueId& operator=(UniqueId&& other) noexcept = default;

		/**
		 * @brief Convert to the underlying value.
		 * @return The underlying integer.
		 */
		constexpr operator Value() const noexcept {
			return value;
		}

		/**
		 * @brief Test whether this identifier is non-zero.
		 * @return True if the underlying value is not zero.
		 */
		constexpr explicit operator bool() const noexcept {
			return value;
		}

		// prefix increment / decrement

		/**
		 * @brief Advance to the next identifier.
		 * @return A reference to this instance after incrementing.
		 */
		constexpr UniqueId& operator++() noexcept {
			++value;
			return *this;
		}

		/**
		 * @brief Step back to the previous identifier.
		 * @return A reference to this instance after decrementing.
		 */
		constexpr UniqueId& operator--() noexcept {
			--value;
			return *this;
		}

		// postfix increment / decrement

		/**
		 * @brief Advance to the next identifier.
		 * @return The identifier as it was before incrementing.
		 */
		constexpr UniqueId operator++(int) noexcept {
			const UniqueId tmp = *this;
			++value;
			return tmp;
		}

		/**
		 * @brief Step back to the previous identifier.
		 * @return The identifier as it was before decrementing.
		 */
		constexpr UniqueId operator--(int) noexcept {
			const UniqueId tmp = *this;
			--value;
			return tmp;
		}

		// compound assign helpers

		/**
		 * @brief Advance by a distance.
		 * @param d How far to advance.
		 * @return A reference to this instance after advancing.
		 */
		constexpr UniqueId& operator+=(Value d) noexcept {
			value += d;
			return *this;
		}

		/**
		 * @brief Step back by a distance.
		 * @param d How far to step back.
		 * @return A reference to this instance after stepping back.
		 */
		constexpr UniqueId& operator-=(Value d) noexcept {
			value -= d;
			return *this;
		}

		/**
		 * @brief Compare two identifiers for equality.
		 * @param other The identifier to compare against.
		 * @return True if both have the same underlying value.
		 */
		constexpr bool operator==(UniqueId other) const noexcept {
			return value == other.value;
		}

		/**
		 * @brief Order two identifiers.
		 * @param other The identifier to compare against.
		 * @return The relative ordering of their underlying values.
		 */
		constexpr auto operator<=>(UniqueId other) const noexcept {
			return value <=> other.value;
		}

		/**
		 * @brief Attach a debug name to this identifier.
		 * @param str The name, which has to outlive the identifier.
		 * @return A reference to this instance.
		 * @note For debugging only; the name takes part in no comparison and no logic.
		 */
		constexpr UniqueId& operator=(const char* str) {
			name = str;
			return *this;
		}

	private:
		Value value{};  //!< The underlying identifier.
		const char* name{};  // Debug-only pointer.
	};

	/**
	 * @enum ExtensionType
	 * @brief Represents the type of an extension in the Plugify ecosystem.
	 *
	 * This enum is used to differentiate between various types of extensions,
	 * such as modules and plugins.
	 */
	enum class ExtensionType {
		Unknown,  ///< The type of the extension is unknown.
		Module,   ///< The extension is a module.
		Plugin    ///< The extension is a plugin.
	};

	/**
	 * @typedef Result
	 * @brief Represents the result of an operation, encapsulating either a value or an error
	 * message.
	 *
	 * This type is a template alias for `std::expected`, which provides a way to handle
	 * operations that may succeed or fail with an error message.
	 *
	 * @tparam T The type of the value to be returned on success.
	 */
	template <typename T>
	using Result = std::expected<T, std::string>;

	/**
	 * @brief Build a failed @ref Result from a ready message.
	 * @tparam First A string-like type.
	 * @param error The error message.
	 * @return An unexpected value carrying the message, assignable to any @ref Result.
	 */
	template <plg::string_like First>
	constexpr auto MakeError(First&& error) {
		return std::unexpected(std::forward<First>(error));
	}

	/**
	 * @brief Build a failed @ref Result from a format string.
	 * @tparam Args Types of the format arguments.
	 * @param fmt Format string for the error message.
	 * @param args Format arguments.
	 * @return An unexpected value carrying the formatted message, assignable to any @ref Result.
	 */
	template <typename... Args>
	constexpr auto MakeError(std::format_string<Args...> fmt, Args&&... args) {
		return std::unexpected(std::format(fmt, std::forward<Args>(args)...));
	}
}

namespace std {
	/**
	 * @brief Hash support for @ref plugify::UniqueId, so it can key a standard container.
	 */
	template <>
	struct hash<plugify::UniqueId> {
		/**
		 * @brief Hash an identifier.
		 * @param id The identifier to hash.
		 * @return The hash of its underlying value.
		 */
		std::size_t operator()(plugify::UniqueId id) const noexcept {
			return std::hash<plugify::UniqueId::Value>{}(id);
		}
	};
}

#ifdef FMT_HEADER_ONLY
namespace fmt {
#else
namespace std {
#endif
	/**
	 * @brief Formatting support for @ref plugify::UniqueId, which prints as its underlying value.
	 */
	template <>
	struct formatter<plugify::UniqueId> {
		/**
		 * @brief Parse the format specification, which has to be empty.
		 * @param ctx The parse context.
		 * @return Iterator to the end of the parsed specification.
		 */
		constexpr auto parse(std::format_parse_context& ctx) {
			return ctx.begin();
		}

		/**
		 * @brief Write an identifier out.
		 * @tparam FormatContext The format context type.
		 * @param id The identifier to format.
		 * @param ctx The format context.
		 * @return Iterator past the written output.
		 */
		template <class FormatContext>
		auto format(const plugify::UniqueId& id, FormatContext& ctx) const {
			return format_to(ctx.out(), "{}", plugify::UniqueId::Value{ id });
		}
	};
}
