#include <catch_amalgamated.hpp>

#include "app/variant_tester.hpp"

#include <plugify/variant.hpp>

TEST_CASE("variant > get_if > index", "[variant]") {
	SECTION("const get_if(variant<Types...>* v) noexcept") {
		{
			using V = plg::variant<int>;
			constexpr const V* v = nullptr;
			static_assert(plg::get_if<0>(v) == nullptr);
		}
		{
			using V = plg::variant<int, const long>;
			constexpr V v(42);
			REQUIRE(plg::get_if<0>(&v));
			static_assert(std::is_same_v<decltype(plg::get_if<0>(&v)), const int*>);
			static_assert(*plg::get_if<0>(&v) == 42);
			static_assert(plg::get_if<1>(&v) == nullptr);
		}
		{
			using V = plg::variant<int, const long>;
			constexpr V v(42l);
			static_assert(std::is_same_v<decltype(plg::get_if<1>(&v)), const long*>);
			static_assert(*plg::get_if<1>(&v) == 42);
			static_assert(plg::get_if<0>(&v) == nullptr);
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int&>;
			int x = 42;
			const V v(x);
			static_assert(std::is_same_v<decltype(plg::get_if<0>(&v)), int*);
			REQUIRE(plg::get_if<0>(&v) == &x);
		}
		{
			using V = plg::variant<int&&>;
			int x = 42;
			const V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get_if<0>(&v)), int*);
			REQUIRE(plg::get_if<0>(&v) == &x);
		}
		{
			using V = plg::variant<const int&&>;
			int x = 42;
			const V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get_if<0>(&v)), const int*);
			REQUIRE(plg::get_if<0>(&v) == &x);
		}
#endif
	}
	SECTION("get_if(variant<Types...>* v) noexcept") {
		{
			using V = plg::variant<int>;
			V* v = nullptr;
			REQUIRE(plg::get_if<0>(v) == nullptr);
		}
		{
			using V = plg::variant<int, long>;
			V v(42);
			REQUIRE(plg::get_if<0>(&v));
			static_assert(std::is_same_v<decltype(plg::get_if<0>(&v)), int*>);
			REQUIRE(*plg::get_if<0>(&v) == 42);
			REQUIRE(plg::get_if<1>(&v) == nullptr);
		}
		{
			using V = plg::variant<int, const long>;
			V v(42l);
			static_assert(std::is_same_v<decltype(plg::get_if<1>(&v)), const long*>);
			REQUIRE(*plg::get_if<1>(&v) == 42);
			REQUIRE(plg::get_if<0>(&v) == nullptr);
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int&>;
			int x = 42;
			V v(x);
			static_assert(std::is_same_v<decltype(plg::get_if<0>(&v)), int*>);
			REQUIRE(plg::get_if<0>(&v) == &x);
		}
		{
			using V = plg::variant<const int&>;
			int x = 42;
			V v(x);
			static_assert(std::is_same_v<decltype(plg::get_if<0>(&v)), const int*>);
			REQUIRE(plg::get_if<0>(&v) == &x);
		}
		{
			using V = plg::variant<int&&>;
			int x = 42;
			V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get_if<0>(&v)), int*>);
			REQUIRE(plg::get_if<0>(&v) == &x);
		}
		{
			using V = plg::variant<const int&&>;
			int x = 42;
			V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get_if<0>(&v)), const int*>);
			REQUIRE(plg::get_if<0>(&v) == &x);
		}
#endif
	}
}
