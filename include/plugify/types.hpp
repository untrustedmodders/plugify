#pragma once

#include <optional>
#include <source_location>
#include <string>
#include <vector>

// #include "plg/uuid.hpp"
#include "plg/enum.hpp"
#include "plg/expected.hpp"
#include "plg/inplace_vector.hpp"
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
	 * @typedef UniqueId
	 * @brief Represents a unique identifier for everything.
	 */
	struct UniqueId {
		using Value = std::ptrdiff_t;

		constexpr UniqueId() noexcept = default;

		constexpr explicit UniqueId(Value v) noexcept
		    : value(v) {
		}

		// conversion to underlying value
		operator auto() const noexcept {
			return value;
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
			UniqueId old{ value };
			++value;
			return old;
		}

		constexpr UniqueId operator--(int) noexcept {
			UniqueId old{ value };
			--value;
			return old;
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

		constexpr void SetName(const std::string& str) noexcept {
			name = str.c_str();
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
	PLUGIFY_FORCE_INLINE auto MakeError(First&& error) {
		return std::unexpected(std::forward<First>(error));
	}

	template <typename... Args>
	PLUGIFY_FORCE_INLINE auto MakeError(std::format_string<Args...> fmt, Args&&... args) {
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
