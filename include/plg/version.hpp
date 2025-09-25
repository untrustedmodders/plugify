#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <limits>
#include <optional>
#include <variant>
#if __has_include(<charconv>)
#include <charconv>
#else
#include <system_error>
#endif

#include "plg/config.hpp"
#include "plg/hash.hpp"
#include "plg/string.hpp"
#include "plg/vector.hpp"

#ifndef PLUGIFY_VECTOR_NO_STD_FORMAT
#include "plg/format.hpp"
#endif

// from https://github.com/Neargye/semver
namespace plg {
	namespace detail {
		template <typename T, typename = void>
		struct resize_uninitialized {
			constexpr static auto resize(T& str, std::size_t size) -> std::void_t<decltype(str.resize(size))> {
				str.resize(size);
			}
		};

		template <typename T>
		struct resize_uninitialized<T, std::void_t<decltype(std::declval<T>().__resize_default_init(42))>> {
			constexpr static void resize(T& str, std::size_t size) {
				str.__resize_default_init(size);
			}
		};

		template <typename Int>
		constexpr std::size_t length(Int n) noexcept {
			std::size_t digits = 0;
			do {
				digits++;
				n /= 10;
			} while (n != 0);
			return digits;
		}

		template <typename OutputIt, typename Int>
		constexpr OutputIt to_chars(OutputIt dest, Int n) noexcept {
			do {
				*(--dest) = static_cast<char>('0' + (n % 10));
				n /= 10;
			} while (n != 0);
			return dest;
		}

		enum struct prerelease_identifier_type {
			numeric,
			alphanumeric
		};

		struct prerelease_identifier {
			prerelease_identifier_type type;
			string identifier;
		};

		class version_parser;
		class prerelease_comparator;
	} // namespace detail

	template <typename I1 = int, typename I2 = int, typename I3 = int>
	class version {
		friend class detail::version_parser;
		friend class detail::prerelease_comparator;

	public:
		using trivially_relocatable = std::conditional_t<
				is_trivially_relocatable<I1>::value &&
				is_trivially_relocatable<I2>::value &&
				is_trivially_relocatable<I3>::value, version, void>;

		constexpr version() = default; // https://semver.org/#how-should-i-deal-with-revisions-in-the-0yz-initial-development-phase
		constexpr version(const version&) = default;
		constexpr version(version&&) = default;
		constexpr ~version() = default;

		constexpr version& operator=(const version&) = default;
		constexpr version& operator=(version&&) = default;

		constexpr I1 major() const noexcept { return _major; }
		constexpr I2 minor() const noexcept { return _minor; }
		constexpr I3 patch() const noexcept { return _patch; }

		constexpr const string& prerelease_tag() const { return _prerelease_tag; }
		constexpr const string& build_metadata() const { return _build_metadata; }

		constexpr string to_string() const;

	private:
		I1 _major = 0;
		I2 _minor = 1;
		I3 _patch = 0;
		string _prerelease_tag;
		string _build_metadata;

		vector<detail::prerelease_identifier> _prerelease_identifiers;

		constexpr std::size_t length() const noexcept {
			return detail::length(_major) + detail::length(_minor) + detail::length(_patch) + 2
				   + (_prerelease_tag.empty() ? 0 : _prerelease_tag.length() + 1)
				   + (_build_metadata.empty() ? 0 : _build_metadata.length() + 1);
		}

		constexpr void clear() noexcept {
			_major = 0;
			_minor = 1;
			_patch = 0;

			_prerelease_tag.clear();
			_prerelease_identifiers.clear();
			_build_metadata.clear();
		}
	};

	template <typename I1, typename I2, typename I3>
	constexpr string version<I1, I2, I3>::to_string() const {
		string result;
		detail::resize_uninitialized<string>{}.resize(result, length());

		auto* it = result.end();
		if (!_build_metadata.empty()) {
			it = std::copy_backward(_build_metadata.begin(), _build_metadata.end(), it);
			*(--it) = '+';
		}

		if (!_prerelease_tag.empty()) {
			it = std::copy_backward(_prerelease_tag.begin(), _prerelease_tag.end(), it);
			*(--it) = '-';
		}

		it = detail::to_chars(it, _patch);
		*(--it) = '.';

		it = detail::to_chars(it, _minor);
		*(--it) = '.';

		it = detail::to_chars(it, _major);

		return result;
	}

#if __has_include(<charconv>)
	struct from_chars_result : std::from_chars_result {
		[[nodiscard]] constexpr operator bool() const noexcept { return ec == std::errc{}; }
	};
#else
	struct from_chars_result {
		const char* ptr;
		std::errc ec;

		[[nodiscard]] constexpr operator bool() const noexcept { return ec == std::errc{}; }
	};
#endif

	enum class version_compare_option : std::uint8_t {
		exclude_prerelease,
		include_prerelease
	};

	namespace detail {
		constexpr from_chars_result success(const char* ptr) noexcept {
			return from_chars_result{ ptr, std::errc{} };
		}

		constexpr from_chars_result failure(const char* ptr, std::errc error_code = std::errc::invalid_argument) noexcept {
			return from_chars_result{ ptr, error_code };
		}

		constexpr bool is_digit(char c) noexcept {
			return c >= '0' && c <= '9';
		}

		constexpr bool is_letter(char c) noexcept {
			return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
		}

		constexpr std::uint8_t to_digit(char c) noexcept {
			return static_cast<std::uint8_t>(c - '0');
		}

		constexpr char to_char(int i) noexcept {
			return '0' + (char)i;
		}

		template<class T, class U>
		constexpr bool cmp_less(T t, U u) noexcept {
			if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
				return t < u;
			else if constexpr (std::is_signed_v<T>)
				return t < 0 || std::make_unsigned_t<T>(t) < u;
			else
				return u >= 0 && t < std::make_unsigned_t<U>(u);
		}

		template<class T, class U>
		constexpr bool cmp_less_equal(T t, U u) noexcept {
			return !cmp_less(u, t);
		}

		template<class T, class U>
		constexpr bool cmp_greater_equal(T t, U u) noexcept {
			return !cmp_less(t, u);
		}

		template<typename R, typename T>
		constexpr bool number_in_range(T t) noexcept {
			return cmp_greater_equal(t, std::numeric_limits<R>::min()) && cmp_less_equal(t, std::numeric_limits<R>::max());
		}

		constexpr int compare(std::string_view lhs, std::string_view rhs) {
			return lhs.compare(rhs);
		}

		constexpr int compare_numerically(std::string_view lhs, std::string_view rhs) {
			// assume that strings don't have leading zeros (we've already checked it at parsing stage).

			if (lhs.size() != rhs.size()) {
				return static_cast<int>(lhs.size() - rhs.size());
			}

			for (std::size_t i = 0; i < lhs.size(); ++i) {
				int a = lhs[i] - '0';
				int b = rhs[i] - '0';
				if (a != b) {
					return a - b;
				}
			}

			return 0;
		}

		enum class token_type : std::uint8_t {
			eol,
			space,
			dot,
			plus,
			hyphen,
			letter,
			digit,
			range_operator,
			logical_or
		};

		enum class range_operator : std::uint8_t {
			less,
			less_or_equal,
			greater,
			greater_or_equal,
			equal
		};

		struct token {
			using value_t = std::variant<std::monostate, std::uint8_t, char, range_operator>;
			token_type type;
			value_t value;
			const char* lexeme;
		};

		class token_stream {
		public:
			constexpr token_stream() = default;
			constexpr explicit token_stream(vector<token> tokens) noexcept : _tokens(std::move(tokens)) {}

			constexpr void push(const token& token) noexcept {
				_tokens.push_back(token);
			}

			constexpr token advance() noexcept {
				const token token = get(_current);
				++_current;
				return token;
			}

			constexpr token peek(std::size_t k = 0) const noexcept {
				return get(_current + k);
			}

			constexpr token previous() const noexcept {
				return get(_current - 1);
			}

			constexpr bool advance_if_match(token& token, token_type type) noexcept {
				if (get(_current).type != type) {
					return false;
				}

				token = advance();
				return true;
			}

			constexpr bool advance_if_match(token_type type) noexcept {
				token token;
				return advance_if_match(token, type);
			}

			constexpr bool consume(token_type type) noexcept {
				return advance().type == type;
			}

			constexpr bool check(token_type type) const noexcept {
				return peek().type == type;
			}

		private:
			std::size_t _current = 0;
			vector<token> _tokens;

			constexpr token get(std::size_t i) const noexcept {
				return _tokens[i];
			}
		};

		class lexer {
		public:
			explicit constexpr lexer(std::string_view text) noexcept : _text{text}, _current_pos{0} {}

			constexpr from_chars_result scan_tokens(token_stream& token_stream) noexcept {
				from_chars_result result{ _text.data(), std::errc{} };

				while (!is_eol()) {
					result = scan_token(token_stream);
					if (!result) {
						return result;
					}
				}

				token_stream.push({ token_type::eol, {}, _text.data() + _text.size() });

				return result;
			}

		private:
			std::string_view _text;
			std::size_t _current_pos;

			constexpr from_chars_result scan_token(token_stream& stream) noexcept {
				const char c = advance();

				switch (c) {
					case ' ':
						add_token(stream, token_type::space);
						break;
					case '.':
						add_token(stream, token_type::dot);
						break;
					case '-':
						add_token(stream, token_type::hyphen);
						break;
					case '+':
						add_token(stream, token_type::plus);
						break;
					case '|':
						if (advance_if_match('|')) {
							add_token(stream, token_type::logical_or);
							break;
						}
						return failure(get_prev_symbol());
					case '<':
						add_token(stream, token_type::range_operator, advance_if_match('=') ? range_operator::less_or_equal : range_operator::less);
						break;
					case '>':
						add_token(stream, token_type::range_operator, advance_if_match('=') ? range_operator::greater_or_equal : range_operator::greater);
						break;
					case '=':
						add_token(stream, token_type::range_operator, range_operator::equal);
						break;
					default:
						if (is_digit(c)) {
							add_token(stream, token_type::digit, to_digit(c));
							break;
						}
						else if (is_letter(c)) {
							add_token(stream, token_type::letter, c);
							break;
						}
						return failure(get_prev_symbol());
				}

				return success(get_prev_symbol());
			}

			constexpr void add_token(token_stream& stream, token_type type, token::value_t value = {}) noexcept {
				const char* lexeme = get_prev_symbol();
				stream.push({ type, value, lexeme});
			}

			constexpr char advance() noexcept {
				char c = _text[_current_pos];
				_current_pos += 1;
				return c;
			}

			constexpr bool advance_if_match(char c) noexcept {
				if (is_eol()) {
					return false;
				}

				if (_text[_current_pos] != c) {
					return false;
				}

				_current_pos += 1;

				return true;
			}

			constexpr const char* get_prev_symbol() const noexcept {
				return _text.data() + _current_pos - 1;
			}

			constexpr bool is_eol() const noexcept { return _current_pos >= _text.size(); }
		};

		class prerelease_comparator {
		public:
			template <typename I1, typename I2, typename I3>
			[[nodiscard]] constexpr int compare(const version<I1, I2, I3>& lhs, const version<I1, I2, I3>& rhs) const noexcept {
				if (lhs._prerelease_identifiers.empty() != rhs._prerelease_identifiers.empty()) {
					return static_cast<int>(rhs._prerelease_identifiers.size()) - static_cast<int>(lhs._prerelease_identifiers.size());
				}

				const std::size_t count = std::min(lhs._prerelease_identifiers.size(), rhs._prerelease_identifiers.size());

				for (std::size_t i = 0; i < count; ++i) {
					const int compare_result = compare_identifier(lhs._prerelease_identifiers[i], rhs._prerelease_identifiers[i]);
					if (compare_result != 0) {
						return compare_result;
					}
				}

				return static_cast<int>(lhs._prerelease_identifiers.size()) - static_cast<int>(rhs._prerelease_identifiers.size());
			}

		private:
			[[nodiscard]] constexpr int compare_identifier(const prerelease_identifier& lhs, const prerelease_identifier& rhs) const noexcept {
				if (lhs.type == prerelease_identifier_type::numeric && rhs.type == prerelease_identifier_type::numeric) {
					return compare_numerically(lhs.identifier, rhs.identifier);
				} else if (lhs.type == prerelease_identifier_type::alphanumeric && rhs.type == prerelease_identifier_type::alphanumeric) {
					return detail::compare(lhs.identifier, rhs.identifier);
				}

				return lhs.type == prerelease_identifier_type::alphanumeric ? 1 : -1;
			}
		};

		class version_parser {
		public:
			constexpr explicit version_parser(token_stream& stream) : _stream{stream} {
			}

			template <typename I1, typename I2, typename I3>
			constexpr from_chars_result parse(version<I1, I2, I3>& out) noexcept {
				out.clear();

				from_chars_result result = parse_number(out._major);
				if (!result) {
					return result;
				}

				if (!_stream.consume(token_type::dot)) {
					return failure(_stream.previous().lexeme);
				}

				result = parse_number(out._minor);
				if (!result) {
					return result;
				}

				if (!_stream.consume(token_type::dot)) {
					return failure(_stream.previous().lexeme);
				}

				result = parse_number(out._patch);
				if (!result) {
					return result;
				}

				if (_stream.advance_if_match(token_type::hyphen)) {
					result = parse_prerelease_tag(out._prerelease_tag, out._prerelease_identifiers);
					if (!result) {
						return result;
					}
				}

				if (_stream.advance_if_match(token_type::plus)) {
					result = parse_build_metadata(out._build_metadata);
					if (!result) {
						return result;
					}
				}

				return result;
			}


		private:
			token_stream& _stream;

			template <typename Int>
			constexpr from_chars_result parse_number(Int& out) {
				token token = _stream.advance();

				if (!is_digit(token)) {
					return failure(token.lexeme);
				}

				const auto first_digit = std::get<std::uint8_t>(token.value);
				std::uint64_t result = first_digit;

				if (first_digit == 0) {
					out = static_cast<Int>(result);
					return success(_stream.peek().lexeme);
				}

				while (_stream.advance_if_match(token, token_type::digit)) {
					result = result * 10 + std::get<std::uint8_t>(token.value);
				}

				if (detail::number_in_range<Int>(result)) {
					out = static_cast<Int>(result);
					return success(_stream.peek().lexeme);
				}

				return failure(token.lexeme, std::errc::result_out_of_range);
			}

			constexpr from_chars_result parse_prerelease_tag(string& out, vector<detail::prerelease_identifier>& out_identifiers) {
				string result;

				do {
					if (!result.empty()) {
						result.push_back('.');
					}

					string identifier;
					if (const auto res = parse_prerelease_identifier(identifier); !res) {
						return res;
					}

					result.append(identifier);
					out_identifiers.push_back(make_prerelease_identifier(identifier));

				} while (_stream.advance_if_match(token_type::dot));

				out = result;
				return success(_stream.peek().lexeme);
			}

			constexpr from_chars_result parse_build_metadata(string& out) {
				string result;

				do {
					if (!result.empty()) {
						result.push_back('.');
					}

					string identifier;
					if (const auto res = parse_build_identifier(identifier); !res) {
						return res;
					}

					result.append(identifier);
				} while (_stream.advance_if_match(token_type::dot));

				out = result;
				return success(_stream.peek().lexeme);
			}

			constexpr from_chars_result parse_prerelease_identifier(string& out) {
				string result;
				token token = _stream.advance();

				do {
					switch (token.type) {
						case token_type::hyphen:
							result.push_back('-');
							break;
						case token_type::letter:
							result.push_back(std::get<char>(token.value));
							break;
						case token_type::digit:
						{
							const auto digit = std::get<std::uint8_t>(token.value);

							// numerical prerelease identifier doesn't allow leading zero
							// 1.2.3-1.alpha is valid,
							// 1.2.3-01b is valid as well, but
							// 1.2.3-01.alpha is not valid

							// Only check for leading zero when digit is the first character of the
							// prerelease identifier.
							if (result.empty() && is_leading_zero(digit)) {
								return failure(token.lexeme);
							}

							result.push_back(to_char(digit));
							break;
						}
						default:
							return failure(token.lexeme);
					}
				} while (_stream.advance_if_match(token, token_type::hyphen) || _stream.advance_if_match(token, token_type::letter) || _stream.advance_if_match(token, token_type::digit));

				out = result;
				return success(_stream.peek().lexeme);
			}

			constexpr detail::prerelease_identifier make_prerelease_identifier(const string& identifier) {
				auto type = detail::prerelease_identifier_type::numeric;
				for (char c : identifier) {
					if (c == '-' || detail::is_letter(c)) {
						type = detail::prerelease_identifier_type::alphanumeric;
						break;
					}
				}
				return detail::prerelease_identifier{ type, identifier };
			}

			constexpr from_chars_result parse_build_identifier(string& out) {
				string result;
				token token = _stream.advance();

				do {
					switch (token.type) {
						case token_type::hyphen:
							result.push_back('-');
							break;
						case token_type::letter:
							result.push_back(std::get<char>(token.value));
							break;
						case token_type::digit:
						{
							const auto digit = std::get<std::uint8_t>(token.value);
							result.push_back(to_char(digit));
							break;
						}
						default:
							return failure(token.lexeme);
					}
				} while (_stream.advance_if_match(token, token_type::hyphen) || _stream.advance_if_match(token, token_type::letter) || _stream.advance_if_match(token, token_type::digit));

				out = result;
				return success(_stream.peek().lexeme);
			}

			constexpr bool is_leading_zero(int digit) noexcept {
				if (digit != 0) {
					return false;
				}

				size_t k = 0;
				int alpha_numerics = 0;
				int digits = 0;

				while (true) {
					const token token = _stream.peek(k);

					if (!is_alphanumeric(token)) {
						break;
					}

					++alpha_numerics;

					if (is_digit(token)) {
						++digits;
					}

					++k;
				}

				return digits > 0 && digits == alpha_numerics;
			}

			constexpr bool is_digit(const token& token) const noexcept {
				return token.type == token_type::digit;
			}

			constexpr bool is_eol(const token& token) const noexcept {
				return token.type == token_type::eol;
			}

			constexpr bool is_alphanumeric(const token& token) const noexcept {
				return token.type == token_type::hyphen || token.type == token_type::letter || token.type == token_type::digit;
			}
		};

		template <typename I1, typename I2, typename I3>
		constexpr int compare_prerelease(const version<I1, I2, I3>& lhs, const version<I1, I2, I3>& rhs) noexcept {
			return prerelease_comparator{}.compare(lhs, rhs);
		}

		template <typename I1, typename I2, typename I3>
		constexpr int compare_parsed(const version<I1, I2, I3>& lhs, const version<I1, I2, I3>& rhs, version_compare_option compare_option) {
			int result = lhs.major() - rhs.major();
			if (result != 0) {
				return result;
			}

			result = lhs.minor() - rhs.minor();
			if (result != 0) {
				return result;
			}

			result = lhs.patch() - rhs.patch();
			if (result != 0) {
				return result;
			}

			if (compare_option == version_compare_option::include_prerelease) {
				result = detail::compare_prerelease(lhs, rhs);
			}

			return result;
		}

		template <typename I1, typename I2, typename I3>
		constexpr from_chars_result parse(std::string_view str, version<I1, I2, I3>& out) {
			token_stream token_stream;
			from_chars_result result = lexer{ str }.scan_tokens(token_stream);
			if (!result) {
				return result;
			}

			result = version_parser{ token_stream }.parse(out);
			if (!result) {
				return result;
			}

			if (!token_stream.consume(token_type::eol)) {
				return failure(token_stream.previous().lexeme);
			}

			return success(token_stream.previous().lexeme);
		}

	} // namespace detail

	template <typename I1, typename I2, typename I3>
	[[nodiscard]] constexpr bool operator==(const version<I1, I2, I3>& lhs, const version<I1, I2, I3>& rhs) noexcept {
		return detail::compare_parsed(lhs, rhs, version_compare_option::include_prerelease) == 0;
	}

	template <typename I1, typename I2, typename I3>
	[[nodiscard]] constexpr bool operator!=(const version<I1, I2, I3>& lhs, const version<I1, I2, I3>& rhs) noexcept {
		return detail::compare_parsed(lhs, rhs, version_compare_option::include_prerelease) != 0;
	}

	template <typename I1, typename I2, typename I3>
	[[nodiscard]] constexpr bool operator>(const version<I1, I2, I3>& lhs, const version<I1, I2, I3>& rhs) noexcept {
		return detail::compare_parsed(lhs, rhs, version_compare_option::include_prerelease) > 0;
	}

	template <typename I1, typename I2, typename I3>
	[[nodiscard]] constexpr bool operator>=(const version<I1, I2, I3>& lhs, const version<I1, I2, I3>& rhs) noexcept {
		return detail::compare_parsed(lhs, rhs, version_compare_option::include_prerelease) >= 0;
	}

	template <typename I1, typename I2, typename I3>
	[[nodiscard]] constexpr bool operator<(const version<I1, I2, I3>& lhs, const version<I1, I2, I3>& rhs) noexcept {
		return detail::compare_parsed(lhs, rhs, version_compare_option::include_prerelease) < 0;
	}

	template <typename I1, typename I2, typename I3>
	[[nodiscard]] constexpr bool operator<=(const version<I1, I2, I3>& lhs, const version<I1, I2, I3>& rhs) noexcept {
		return detail::compare_parsed(lhs, rhs, version_compare_option::include_prerelease) <= 0;
	}

	template <typename I1, typename I2, typename I3>
	[[nodiscard]] constexpr std::strong_ordering operator<=>(const version<I1, I2, I3>& lhs, const version<I1, I2, I3>& rhs) {
		int compare = detail::compare_parsed(lhs, rhs, version_compare_option::include_prerelease);
		if (compare == 0)
			return std::strong_ordering::equal;
  		if (compare > 0)
			return std::strong_ordering::greater;
  		return std::strong_ordering::less;
	}

	template<typename I1, typename I2, typename I3>
	constexpr from_chars_result parse(std::string_view str, version<I1, I2, I3>& output) {
		return detail::parse(str, output);
	}

	constexpr bool valid(std::string_view str) {
		version v{};
		return detail::parse(str, v);
	}

	namespace detail {
		template <typename I1, typename I2, typename I3>
		class range_comparator {
		public:
			constexpr range_comparator(const version<I1, I2, I3>& v, range_operator op) noexcept : _v(v), _op(op) {}

			constexpr bool contains(const version<I1, I2, I3>& other) const noexcept {
				switch (_op) {
					case range_operator::less:
						return detail::compare_parsed(other, _v, version_compare_option::include_prerelease) < 0;
					case range_operator::less_or_equal:
						return detail::compare_parsed(other, _v, version_compare_option::include_prerelease) <= 0;
					case range_operator::greater:
						return detail::compare_parsed(other, _v, version_compare_option::include_prerelease) > 0;
					case range_operator::greater_or_equal:
						return detail::compare_parsed(other, _v, version_compare_option::include_prerelease) >= 0;
					case range_operator::equal:
						return detail::compare_parsed(other, _v, version_compare_option::include_prerelease) == 0;
				}
				return false;
			}

			constexpr const version<I1, I2, I3>& get_version() const noexcept { return _v; }

			constexpr range_operator get_operator() const noexcept { return _op; }

			constexpr string to_string() const {
				string result;
				switch (_op) {
					case range_operator::less: result += "<"; break;
					case range_operator::less_or_equal: result += "<="; break;
					case range_operator::greater: result += ">"; break;
					case range_operator::greater_or_equal: result += ">="; break;
					case range_operator::equal: result += "="; break;
				}
				result += _v.to_string();
				return result;
			}

		private:
			version<I1, I2, I3> _v;
			range_operator _op;
		};

		class range_parser;

		template <typename I1, typename I2, typename I3>
		class range {
		public:
			friend class detail::range_parser;

			constexpr bool contains(const version<I1, I2, I3>& v, version_compare_option option) const noexcept {
				if (option == version_compare_option::exclude_prerelease) {
					if (!match_at_least_one_comparator_with_prerelease(v)) {
						return false;
					}
				}

				return std::all_of(_ranges_comparators.begin(), _ranges_comparators.end(), [&](const auto& ranges_comparator) {
					return ranges_comparator.contains(v);
				});
			}

			constexpr auto begin() const noexcept {
				return _ranges_comparators.begin();
			}

			constexpr auto end() const noexcept {
				return _ranges_comparators.end();
			}

			constexpr std::size_t size() const noexcept {
				return _ranges_comparators.size();
			}

			constexpr bool empty() const noexcept {
				return _ranges_comparators.empty();
			}

			constexpr string to_string() const {
				return join(_ranges_comparators, " ");
			}

		private:
			vector<range_comparator<I1, I2, I3>> _ranges_comparators;

			constexpr bool match_at_least_one_comparator_with_prerelease(const version<I1, I2, I3>& v) const noexcept {
				if (v.prerelease_tag().empty()) {
					return true;
				}

				return std::any_of(_ranges_comparators.begin(), _ranges_comparators.end(), [&](const auto& ranges_comparator) {
					const bool has_prerelease = !ranges_comparator.get_version().prerelease_tag().empty();
					const bool equal_without_prerelease = detail::compare_parsed(v, ranges_comparator.get_version(), version_compare_option::exclude_prerelease) == 0;
					return has_prerelease && equal_without_prerelease;
				});
			}
		};
	}

	template <typename I1 = int, typename I2 = int, typename I3 = int>
	class range_set {
	public:
		friend class detail::range_parser;

		constexpr bool contains(const version<I1, I2, I3>& v, version_compare_option option = version_compare_option::exclude_prerelease) const noexcept {
			return std::any_of(_ranges.begin(), _ranges.end(), [&](const auto& range) {
				return range.contains(v, option);
			});
		}

		constexpr auto begin() const noexcept {
			return _ranges.begin();
		}

		constexpr auto end() const noexcept {
			return _ranges.end();
		}

		constexpr std::size_t size() const noexcept {
			return _ranges.size();
		}

		constexpr bool empty() const noexcept {
			return _ranges.empty();
		}

		constexpr string to_string() const {
			return join(_ranges, " ");
		}

	private:
		vector<detail::range<I1, I2, I3>> _ranges;
	};

	namespace detail {
		class range_parser {
		public:
			constexpr explicit range_parser(token_stream stream) noexcept : _stream(std::move(stream)) {}

			template <typename I1, typename I2, typename I3>
			constexpr from_chars_result parse(range_set<I1, I2, I3>& out) noexcept {
				vector<range<I1, I2, I3>> ranges;

				do {

					detail::range<I1, I2, I3> range;
					if (const auto res = parse_range(range); !res) {
						return res;
					}

					ranges.push_back(range);
					skip_whitespaces();

				} while (_stream.advance_if_match(token_type::logical_or));

				out._ranges = std::move(ranges);

				return success(_stream.peek().lexeme);
			}

		private:
			token_stream _stream;

			template <typename I1, typename I2, typename I3>
			constexpr from_chars_result parse_range(detail::range<I1, I2, I3>& out) noexcept {
				do {
					skip_whitespaces();

					if (const auto res = parse_range_comparator(out._ranges_comparators); !res) {
						return res;
					}

					skip_whitespaces();

				} while (_stream.check(token_type::range_operator) || _stream.check(token_type::digit));

				return success(_stream.peek().lexeme);
			}

			template <typename I1, typename I2, typename I3>
			constexpr from_chars_result parse_range_comparator(vector<detail::range_comparator<I1, I2, I3>>& out) noexcept {
				range_operator op = range_operator::equal;
				token token;
				if (_stream.advance_if_match(token, token_type::range_operator)) {
					op = std::get<range_operator>(token.value);
				}

				skip_whitespaces();

				version<I1, I2, I3> ver;
				version_parser parser{ _stream };
				if (const auto res = parser.parse(ver); !res) {
					return res;
				}

				out.emplace_back(ver, op);
				return success(_stream.peek().lexeme);
			}

			constexpr void skip_whitespaces() noexcept {
				while (_stream.advance_if_match(token_type::space)) {
					;
				}
			}
		};
	} // namespace detail


	template <typename I1, typename I2, typename I3>
	constexpr from_chars_result parse(std::string_view str, range_set<I1, I2, I3>& out) {
		detail::token_stream token_stream;
		const from_chars_result result =  detail::lexer{ str }.scan_tokens(token_stream);
		if (!result) {
			return result;
		}

		return detail::range_parser{ std::move(token_stream) }.parse(out);
	}
} // namespace plg

#ifndef PLUGIFY_VECTOR_NO_STD_HASH
// hash support
namespace std {
	template<typename I1, typename I2, typename I3>
	struct hash<plg::version<I1, I2, I3>> {
		std::size_t operator()(const plg::version<I1, I2, I3>& ver) const noexcept {
			std::size_t seed = 0;
			plg::hash_combine_all(seed,
				ver.major(),
				ver.minor(),
				ver.patch(),
				ver.prerelease_tag(),
				ver.build_metadata());
			return seed;
		}
	};
}// namespace std
#endif // PLUGIFY_VECTOR_NO_STD_HASH

#ifndef PLUGIFY_VECTOR_NO_STD_FORMAT
// format support
#ifdef FMT_HEADER_ONLY
namespace fmt {
#else
namespace std {
#endif
	template<typename I1, typename I2, typename I3>
	struct formatter<plg::version<I1, I2, I3>> {
		constexpr auto parse(std::format_parse_context& ctx) {
			return ctx.begin();
		}

		template<class FormatContext>
		auto format(const plg::version<I1, I2, I3>& ver, FormatContext& ctx) const {
			return std::format_to(ctx.out(), "{}", ver.to_string());
		}
	};
	template<typename I1, typename I2, typename I3>
	struct formatter<plg::range_set<I1, I2, I3>> {
		constexpr auto parse(std::format_parse_context& ctx) {
			return ctx.begin();
		}

		template<class FormatContext>
		auto format(const plg::range_set<I1, I2, I3>& ver, FormatContext& ctx) const {
			return std::format_to(ctx.out(), "{}", ver.to_string());
		}
	};
	template<typename I1, typename I2, typename I3>
	struct formatter<plg::detail::range<I1, I2, I3>> {
		constexpr auto parse(std::format_parse_context& ctx) {
			return ctx.begin();
		}

		template<class FormatContext>
		auto format(const plg::detail::range<I1, I2, I3>& ver, FormatContext& ctx) const {
			return std::format_to(ctx.out(), "{}", ver.to_string());
		}
	};
	template<typename I1, typename I2, typename I3>
	struct formatter<plg::detail::range_comparator<I1, I2, I3>> {
		constexpr auto parse(std::format_parse_context& ctx) {
			return ctx.begin();
		}

		template<class FormatContext>
		auto format(const plg::detail::range_comparator<I1, I2, I3>& ver, FormatContext& ctx) const {
			return std::format_to(ctx.out(), "{}", ver.to_string());
		}
	};
}// namespace std
#endif // PLUGIFY_VECTOR_NO_STD_FORMAT
