#include <catch_amalgamated.hpp>

#include "app/variant_tester.hpp"

#include <plugify/variant.hpp>

namespace {
	template<std::size_t I>
	using Idx = std::integral_constant<size_t, I>;
} // namespace

TEST_CASE("variant > get > index", "[variant]") {

	SECTION("variant_alternative_t<I, variant<Types...>>& get(variant<Types...>& v)") {
		{
			using V = plg::variant<int, const long>;
			V v(42);
			REQUIRE(plg::get<0>(v));
			static_assert(std::is_same_v<decltype(plg::get<0>(v)), int&>);
			REQUIRE(plg::get<0>(v) == 42);
		}
		{
			using V = plg::variant<int, const long>;
			V v(42l);
			static_assert(std::is_same_v<decltype(plg::get<1>(v)), const long&>);
			REQUIRE(plg::get<1>(v) == 42);
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int&>;
			int x = 42;
			V v(x);
			static_assert(std::is_same_v<decltype(plg::get<0>(v)), int&>);
			REQUIRE(&plg::get<0>(v) == &x);
		}
		{
			using V = plg::variant<const int&>;
			int x = 42;
			V v(x);
			static_assert(std::is_same_v<decltype(plg::get<0>(v)), const int&>);
			REQUIRE(&plg::get<0>(v) == &x);
		}
		{
			using V = plg::variant<int&&>;
			int x = 42;
			V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get<0>(v)), int&>);
			REQUIRE(&plg::get<0>(v) == &x);
		}
		{
			using V = plg::variant<const int&&>;
			int x = 42;
			V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get<0>(v)), const int&>);
			REQUIRE(&plg::get<0>(v) == &x);
		}
#endif
	}
	SECTION("variant_alternative_t<I, variant<Types...>>&& get(variant<Types...>&& v)") {
		{
			using V = plg::variant<int, const long>;
			V v(42);
			REQUIRE(plg::get<0>(std::move(v)));
			static_assert(std::is_same_v<decltype(plg::get<0>(std::move(v))), int&&>);
			REQUIRE(plg::get<0>(std::move(v)) == 42);
			}
			{
				using V = plg::variant<int, const long>;
				V v(42l);
				static_assert(std::is_same_v<decltype(plg::get<1>(std::move(v))), const long&&>);
				REQUIRE(plg::get<1>(std::move(v)) == 42);
			}
#if !TEST_VARIANT_HAS_NO_REFERENCES
			{
				using V = plg::variant<int&>;
				int x = 42;
				V v(x);
				static_assert(std::is_same_v<decltype(plg::get<0>(std::move(v))), int&>);
				REQUIRE(&plg::get<0>(std::move(v)) == &x);
			}
			{
				using V = plg::variant<const int&>;
				int x = 42;
				V v(x);
				static_assert(std::is_same_v<decltype(plg::get<0>(std::move(v))), const int&>);
				REQUIRE(&plg::get<0>(std::move(v)) == &x);
			}
			{
				using V = plg::variant<int&&>;
				int x = 42;
				V v(std::move(x));
				static_assert(std::is_same_v<decltype(plg::get<0>(std::move(v))), int&&>);
				int&& xref = plg::get<0>(std::move(v));
				REQUIRE(&xref == &x);
			}
			{
				using V = plg::variant<const int&&>;
				int x = 42;
				V v(std::move(x));
				static_assert(std::is_same_v<decltype(plg::get<0>(std::move(v))), const int&&>);
				const int&& xref = plg::get<0>(std::move(v));
				REQUIRE(&xref == &x);
			}
#endif
	}
	SECTION("variant_alternative_t<I, variant<Types...>> const& get(const variant<Types...>& v)") {
		{
			using V = plg::variant<int, const long>;
			constexpr V v(42);
		#ifdef TEST_WORKAROUND_CONSTEXPR_IMPLIES_NOEXCEPT
			REQUIRE(plg::get<0>(v));
		#else
			REQUIRE(plg::get<0>(v));
		#endif
			static_assert(std::is_same_v<decltype(plg::get<0>(v)), const int&>);
			static_assert(plg::get<0>(v) == 42);
		}
		{
			using V = plg::variant<int, const long>;
			const V v(42);
			REQUIRE(plg::get<0>(v));
			static_assert(std::is_same_v<decltype(plg::get<0>(v)), const int&>);
			REQUIRE(plg::get<0>(v) == 42);
		}
		{
			using V = plg::variant<int, const long>;
			constexpr V v(42l);
		#ifdef TEST_WORKAROUND_CONSTEXPR_IMPLIES_NOEXCEPT
			REQUIRE(plg::get<1>(v));
		#else
			REQUIRE(plg::get<1>(v));
		#endif
			static_assert(std::is_same_v<decltype(plg::get<1>(v)), const long&>);
			static_assert(plg::get<1>(v) == 42);
		}
		{
			using V = plg::variant<int, const long>;
			const V v(42l);
			REQUIRE(plg::get<1>(v));
			static_assert(std::is_same_v<decltype(plg::get<1>(v)), const long&>);
			REQUIRE(plg::get<1>(v) == 42);
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int&>;
			int x = 42;
			const V v(x);
			static_assert(std::is_same_v<decltype(plg::get<0>(v)), int&>);
			REQUIRE(&plg::get<0>(v) == &x);
		}
		{
			using V = plg::variant<int&&>;
			int x = 42;
			const V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get<0>(v)), int&>);
			REQUIRE(&plg::get<0>(v) == &x);
		}
		{
			using V = plg::variant<const int&&>;
			int x = 42;
			const V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get<0>(v)), const int&>);
			REQUIRE(&plg::get<0>(v) == &x);
		}
		#endif
	}
	SECTION("variant_alternative_t<I, variant<Types...>> const&& get(const variant<Types...>&& v)") {
		{
			using V = plg::variant<int, const long>;
			const V v(42);
			REQUIRE(plg::get<0>(std::move(v)));
			static_assert(std::is_same_v<decltype(plg::get<0>(std::move(v))), const int&&>);
			REQUIRE(plg::get<0>(std::move(v)) == 42);
		}
		{
			using V = plg::variant<int, const long>;
			const V v(42l);
			static_assert(std::is_same_v<decltype(plg::get<1>(std::move(v))), const long&&>);
			REQUIRE(plg::get<1>(std::move(v)) == 42);
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int&>;
			int x = 42;
			const V v(x);
			static_assert(std::is_same_v<decltype(plg::get<0>(std::move(v))), int&>);
			REQUIRE(&plg::get<0>(std::move(v)) == &x);
		}
		{
			using V = plg::variant<const int&>;
			int x = 42;
			const V v(x);
			static_assert(std::is_same_v<decltype(plg::get<0>(std::move(v))), const int&>);
			REQUIRE(&plg::get<0>(std::move(v)) == &x);
		}
		{
			using V = plg::variant<int&&>;
			int x = 42;
			const V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get<0>(std::move(v))), int&&>);
			int&& xref = plg::get<0>(std::move(v));
			REQUIRE(&xref == &x);
		}
		{
			using V = plg::variant<const int&&>;
			int x = 42;
			const V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get<0>(std::move(v))), const int&&>);
			const int&& xref = plg::get<0>(std::move(v));
			REQUIRE(&xref == &x);
		}
#endif
	}
	SECTION("test_throws_for_all_value_categories") {
#if TEST_HAS_NO_EXCEPTIONS
		using V = plg::variant<int, long>;
		V v0(42);
		const V& cv0 = v0;
		REQUIRE(v0.index() == 0);
		V v1(42l);
		const V& cv1 = v1;
		REQUIRE(v1.index() == 1);
		std::integral_constant<size_t, 0> zero;
		std::integral_constant<size_t, 1> one;
		auto test = [](auto idx, auto&& v) {
			using Idx = decltype(idx);
			try {
				[[maybe_unused]] auto _ = plg::get<Idx::value>(std::forward<decltype(v)>(v));
			} catch (const plg::bad_variant_access&) {
				return true;
			} catch (...) { /* ... */
			}
			return false;
		};
		{// lvalue test cases
			REQUIRE(test(one, v0));
			REQUIRE(test(zero, v1));
		}
		{// const lvalue test cases
			REQUIRE(test(one, cv0));
			REQUIRE(test(zero, cv1));
		}
		{// rvalue test cases
			REQUIRE(test(one, std::move(v0)));
			REQUIRE(test(zero, std::move(v1)));
		}
		{// const rvalue test cases
			REQUIRE(test(one, std::move(cv0)));
			REQUIRE(test(zero, std::move(cv1)));
		}
#endif
	}
}
