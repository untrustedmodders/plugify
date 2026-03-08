#pragma once

#include <cstdint>
#include <string_view>

#include "plg/config.hpp"

#ifndef PLUGIFY_MODULE_NAME
#define PLUGIFY_MODULE_NAME ""
#endif

namespace plg {
	class source_location {
		const std::size_t line_;
		const std::size_t column_;
		const std::string_view file_name_;
		const std::string_view function_name_;
		const std::string_view module_name_;

	public:
		explicit constexpr source_location(
			const std::size_t line = 0,
			const std::size_t column = 0,
			const std::string_view file_name = "",
			const std::string_view function_name = "",
			const std::string_view module_name = ""
		) noexcept
			: line_(line)
			, column_(column)
			, file_name_(file_name)
			, function_name_(function_name)
			, module_name_(module_name) {
		}

#if __has_builtin(__builtin_COLUMN) || (defined(_MSC_VER) && _MSC_VER > 1925)
		inline static constexpr source_location current(
			const std::size_t line = __builtin_LINE(),
			const std::size_t column = __builtin_COLUMN(),
			const std::string_view file_name = __builtin_FILE(),
			const std::string_view function_name = __builtin_FUNCTION(),
			const std::string_view module_name = PLUGIFY_MODULE_NAME
		) noexcept
#elif defined(__GNUC__) and (__GNUC__ > 4 or (__GNUC__ == 4 and __GNUC_MINOR__ >= 8))
		inline static constexpr source_location current(
			const std::size_t line = __builtin_LINE(),
			const std::size_t column = 0,
			const std::string_view file_name = __builtin_FILE(),
			const std::string_view function_name = __builtin_FUNCTION(),
			const std::string_view module_name = PLUGIFY_MODULE_NAME
		) noexcept
#else
		inline static constexpr source_location current(
			const std::size_t line = 0,
			const std::size_t column = 0,
			const std::string_view file_name = "",
			const std::string_view function_name = ""
			const std::string_view module_name = ""
		) noexcept
#endif
		{
			return source_location(line, column, file_name, function_name, module_name);
		}

		constexpr std::size_t line() const noexcept {
			return line_;
		}

		constexpr std::size_t column() const noexcept {
			return column_;
		}

		constexpr std::string_view file_name() const noexcept {
			return file_name_;
		}

		constexpr std::string_view function_name() const noexcept {
			return function_name_;
		}

		constexpr std::string_view module_name() const noexcept {
			return module_name_;
		}
	};
}