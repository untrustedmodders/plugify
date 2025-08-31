#pragma once

#include <optional>
#include <string>
#include <vector>

//#include "plg/uuid.hpp"
#include "plg/version.hpp"
#include "plg/expected.hpp"
#include "plg/flat_map.hpp"
#include "plg/enum.hpp"

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
        constexpr explicit UniqueId(Value v) noexcept : value(v) {}

        // conversion to underlying value
        operator auto() const noexcept { return value; }

        // prefix increment / decrement
        constexpr UniqueId& operator++() noexcept { ++value; return *this; }
        constexpr UniqueId& operator--() noexcept { --value; return *this; }

        // postfix increment / decrement
        constexpr UniqueId operator++(int) noexcept { UniqueId old{value}; ++value; return old; }
        constexpr UniqueId operator--(int) noexcept { UniqueId old{value}; --value; return old; }

        // compound assign helpers
        constexpr UniqueId& operator+=(Value d) noexcept { value += d; return *this; }
        constexpr UniqueId& operator-=(Value d) noexcept { value -= d; return *this; }

        constexpr bool operator==(UniqueId other) const noexcept { return value == other.value; }
        constexpr auto operator<=>(UniqueId other) const noexcept { return value <=> other.value; }

        constexpr void SetName(const std::string& str) noexcept { name = str.c_str(); }

    private:
        Value value{-1};
        const char* name{nullptr}; // Debug-only pointer.
    };

    /**
     * TODO:
     */
    enum class ExtensionType {
		Unknown,
		Module,
		Plugin
	};

	/**
	 * TODO:
	 */
	template<typename T>
	using Result = plg::expected<T, std::string>;

    /**
     *
     */
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::microseconds;
}

namespace std {
    template<> struct hash<plugify::UniqueId> {
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
    template<>
    struct formatter<plugify::UniqueId> {
        constexpr auto parse(std::format_parse_context& ctx) {
            return ctx.begin();
        }

        template<class FormatContext>
        auto format(const plugify::UniqueId& id, FormatContext& ctx) const {
            return std::format_to(ctx.out(), "{}", plugify::UniqueId::Value{id});
        }
    };
}
