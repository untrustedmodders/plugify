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
	 */
	using Constraint = plg::range_set<>;

	/**
	 * @typedef Location
	 */
	using Location = plg::source_location;

	/**
	 * @typedef UniqueId
	 * @brief Represents a unique identifier for everything.
	 */
	struct UniqueId {
		using Value = std::ptrdiff_t;

		constexpr UniqueId() noexcept = default;
		constexpr explicit UniqueId(Value v) noexcept : value(v) {}
		constexpr ~UniqueId() = default;

		constexpr UniqueId(const UniqueId& other) = default;
		constexpr UniqueId(UniqueId&& other) noexcept = default;
		constexpr UniqueId& operator=(const UniqueId& other) = default;
		constexpr UniqueId& operator=(UniqueId&& other) noexcept = default;

		// conversion to underlying value
		constexpr operator Value() const noexcept {
			return value;
		}

		constexpr explicit operator bool() const noexcept {
			return value != -1;
		}

		// prefix increment / decrement
		constexpr UniqueId& operator++() noexcept {
			++value;
			return *this;
		}

		constexpr UniqueId& operator--() noexcept {
			--value;
			return *this;
		}

		// postfix increment / decrement
		constexpr UniqueId operator++(int) noexcept {
			const UniqueId tmp = *this;
			++value;
			return tmp;
		}

		constexpr UniqueId operator--(int) noexcept {
			const UniqueId tmp = *this;
			--value;
			return tmp;
		}

		// compound assign helpers
		constexpr UniqueId& operator+=(Value d) noexcept {
			value += d;
			return *this;
		}

		constexpr UniqueId& operator-=(Value d) noexcept {
			value -= d;
			return *this;
		}

		constexpr bool operator==(UniqueId other) const noexcept {
			return value == other.value;
		}

		constexpr auto operator<=>(UniqueId other) const noexcept {
			return value <=> other.value;
		}

		// for debug purpose only, not used in any logic
		constexpr UniqueId& operator=(const char* str) {
			name = str;
			return *this;
		}

	private:
		Value value{ -1 };
		const char* name{ nullptr };  // Debug-only pointer.
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

	// Helper for creating errors with context

	template <plg::string_like First>
	constexpr auto MakeError(First&& error) {
		return std::unexpected(std::forward<First>(error));
	}

	template <typename... Args>
	constexpr auto MakeError(std::format_string<Args...> fmt, Args&&... args) {
		return std::unexpected(std::format(fmt, std::forward<Args>(args)...));
	}
}

namespace std {
	template <>
	struct hash<plugify::UniqueId> {
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
	template <>
	struct formatter<plugify::UniqueId> {
		constexpr auto parse(std::format_parse_context& ctx) {
			return ctx.begin();
		}

		template <class FormatContext>
		auto format(const plugify::UniqueId& id, FormatContext& ctx) const {
			return std::format_to(ctx.out(), "{}", plugify::UniqueId::Value{ id });
		}
	};
}
