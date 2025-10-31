#pragma once

#include <algorithm>
#include <array>
#include <cstdio>
#include <iostream>
#include <string_view>

#include "plg/config.hpp"

// from
// https://web.archive.org/web/20230212183620/https://blog.rink.nu/2023/02/12/behind-the-magic-of-magic_enum/
namespace plg {
	constexpr auto ENUM_MIN_VALUE = -128;
	constexpr auto ENUM_MAX_VALUE = 128;

	template <std::size_t N>
	struct static_string {
		constexpr static_string(std::string_view sv) noexcept {
			std::copy(sv.begin(), sv.end(), content_.begin());
		}

		constexpr operator std::string_view() const noexcept {
			return { content_.data(), N };
		}

	private:
		std::array<char, N + 1> content_{};
	};

	constexpr auto pretty_name(std::string_view sv) noexcept {
		// Find last non-pretty character from the end
		auto pos = sv.find_last_not_of(
		    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"
		);

		if (pos != std::string_view::npos) {
			sv.remove_prefix(pos + 1);
		}

		return sv;
	}

	template <typename E, E V>
	constexpr auto n() noexcept {
#if PLUGIFY_COMPILER_GCC || PLUGIFY_COMPILER_CLANG
		return pretty_name({ __PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2 });
#elif PLUGIFY_COMPILER_MSVC
		return pretty_name({ __FUNCSIG__, sizeof(__FUNCSIG__) - 17 });
#endif
	}

	template <typename E, E V>
	constexpr auto is_valid() {
		[[maybe_unused]] constexpr E v = static_cast<E>(V);
		return !n<E, V>().empty();
	}

	template <typename E>
	constexpr auto value(std::size_t v) {
		return static_cast<E>(ENUM_MIN_VALUE + static_cast<int>(v));
	}

	template <std::size_t N>
	constexpr auto count_values(const bool (&valid)[N]) {
		std::size_t count = 0;
		for (std::size_t n = 0; n < N; ++n) {
			if (valid[n]) {
				++count;
			}
		}
		return count;
	}

	template <typename E, std::size_t... I>
	constexpr auto values(std::index_sequence<I...>) noexcept {
		constexpr bool valid[sizeof...(I)] = { is_valid<E, value<E>(I)>()... };
		constexpr auto num_valid = count_values(valid);
		static_assert(num_valid > 0, "no support for empty enums");

		std::array<E, num_valid> values = {};
		for (std::size_t offset = 0, n = 0; n < num_valid; ++offset) {
			if (valid[offset]) {
				values[n] = value<E>(offset);
				++n;
			}
		}

		return values;
	}

	template <typename E>
	constexpr auto values() noexcept {
		constexpr auto enum_size = ENUM_MAX_VALUE - ENUM_MIN_VALUE + 1;
		return values<E>(std::make_index_sequence<enum_size>({}));
	}

	template <typename E>
	inline constexpr auto values_v = values<E>();

	template <typename E, E V>
	constexpr auto enum_name() {
		constexpr auto name = n<E, V>();
		return static_string<name.size()>(name);
	}

	template <typename E, E V>
	inline constexpr auto enum_name_v = enum_name<E, V>();

	template <typename E, std::size_t... I>
	constexpr auto entries(std::index_sequence<I...>) noexcept {
		return std::array<std::pair<E, std::string_view>, sizeof...(I)>{
			{ { values_v<E>[I], enum_name_v<E, values_v<E>[I]> }... }
		};
	}

	template <typename E>
	inline constexpr auto entries_v = entries<E>(std::make_index_sequence<values_v<E>.size()>());

	template <typename E>
	constexpr std::string_view enum_to_string(E value) {
		for (const auto& [key, name] : entries_v<E>) {
			if (value == key) {
				return name;
			}
		}
		return {};
	}
}
