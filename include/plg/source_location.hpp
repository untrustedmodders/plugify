#pragma once

#include <cstdint>
#include <string_view>

#include "plg/config.hpp"

#ifndef PLUGIFY_MODULE
#define PLUGIFY_MODULE ""
#endif

namespace plg {
	class source_location {
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

		inline static constexpr source_location current(
			const std::size_t line = PLUGIFY_LINE,
			const std::size_t column = PLUGIFY_COLUMN,
			const std::string_view file_name = PLUGIFY_FILE,
			const std::string_view function_name = PLUGIFY_FUNCTION,
			const std::string_view module_name = PLUGIFY_MODULE
		) noexcept {
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

	private:
		std::size_t line_;
		std::size_t column_;
		std::string_view file_name_;
		std::string_view function_name_;
		std::string_view module_name_;
	};
}