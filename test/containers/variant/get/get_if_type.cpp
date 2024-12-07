#include <catch_amalgamated.hpp>

#include "app/variant_tester.hpp"

#include <plugify/variant.hpp>

TEST_CASE("variant > get_if > type", "[variant]") {
	SECTION("add_pointer_t<T> get_if(variant<Types...>* v) noexcept") {
		{
			using V = plg::variant<int>;
			constexpr const V* v = nullptr;
			static_assert(plg::get_if<int>(v) == nullptr);
		}
		{
			using V = plg::variant<int, const long>;
			constexpr V v(42);
			REQUIRE(plg::get_if<int>(&v));
			static_assert(std::is_same_v<decltype(plg::get_if<int>(&v)), const int*>);
			static_assert(*plg::get_if<int>(&v) == 42);
			static_assert(plg::get_if<const long>(&v) == nullptr);
		}
		{
			using V = plg::variant<int, const long>;
			constexpr V v(42l);
			static_assert(std::is_same_v<decltype(plg::get_if<const long>(&v)), const long*>);
			static_assert(*plg::get_if<const long>(&v) == 42);
			static_assert(plg::get_if<int>(&v) == nullptr);
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int&>;
			int x = 42;
			const V v(x);
			static_assert(std::is_same_v<decltype(plg::get_if<int&>(&v)), int*v);
			REQUIRE(plg::get_if<int&>(&v) == &x);
		}
		{
			using V = plg::variant<int&&>;
			int x = 42;
			const V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get_if<int&&>(&v)), int*>);
			REQUIRE(plg::get_if<int&&>(&v) == &x);
		}
		{
			using V = plg::variant<const int&&>;
			int x = 42;
			const V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get_if<const int&&>(&v)), const int*>);
			REQUIRE(plg::get_if<const int&&>(&v) == &x);
		}
		#endif
	}
	SECTION("add_pointer_t<const T> get_if(const variant<Types...>* v)") {
		{
			using V = plg::variant<int>;
			V* v = nullptr;
			REQUIRE(plg::get_if<int>(v) == nullptr);
		}
		{
			using V = plg::variant<int, const long>;
			V v(42);
			REQUIRE(plg::get_if<int>(&v));
			static_assert(std::is_same_v<decltype(plg::get_if<int>(&v)), int*>);
			REQUIRE(*plg::get_if<int>(&v) == 42);
			REQUIRE(plg::get_if<const long>(&v) == nullptr);
		}
		{
			using V = plg::variant<int, const long>;
			V v(42l);
			static_assert(std::is_same_v<decltype(plg::get_if<const long>(&v)), const long*>);
			REQUIRE(*plg::get_if<const long>(&v) == 42);
			REQUIRE(plg::get_if<int>(&v) == nullptr);
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int&>;
			int x = 42;
			V v(x);
			static_assert(std::is_same_v<decltype(plg::get_if<int&>(&v)), int*>);
			REQUIRE(plg::get_if<int&>(&v) == &x);
		}
		{
			using V = plg::variant<const int&>;
			int x = 42;
			V v(x);
			static_assert(std::is_same_v<decltype(plg::get_if<const int&>(&v)), const int*>);
			REQUIRE(plg::get_if<const int&>(&v) == &x);
		}
		{
			using V = plg::variant<int&&>;
			int x = 42;
			V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get_if<int&&>(&v)), int*>);
			REQUIRE(plg::get_if<int&&>(&v) == &x);
		}
		{
			using V = plg::variant<const int&&>;
			int x = 42;
			V v(std::move(x));
			static_assert(std::is_same_v<decltype(plg::get_if<const int&&>(&v)), const int*>);
			REQUIRE(plg::get_if<const int&&>(&v) == &x);
		}
#endif
	}
}
