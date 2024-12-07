#include <catch_amalgamated.hpp>

#include <plugify/variant.hpp>

#include "app/test_convertible.hpp"

TEST_CASE("variant index constructor", "[variant]") {
	SECTION("explicit variant(in_place_index_t<I>, Args&&...) > basic") {
		{
			constexpr plg::variant<int> v(plg::in_place_index<0>, 42);
			static_assert(v.index() == 0);
			static_assert(plg::get<0>(v) == 42);
		}
		{
			constexpr plg::variant<int, long, long> v(plg::in_place_index<1>, 42);
			static_assert(v.index() == 1);
			static_assert(plg::get<1>(v) == 42);
		}
		{
			constexpr plg::variant<int, const int, long> v(plg::in_place_index<1>, 42);
			static_assert(v.index() == 1);
			static_assert(plg::get<1>(v) == 42);
		}
		{
			using V = plg::variant<const int, volatile int, int>;
			int x = 42;
			V v(plg::in_place_index<0>, x);
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v) == x);
		}
		{
			using V = plg::variant<const int, volatile int, int>;
			int x = 42;
			V v(plg::in_place_index<1>, x);
			REQUIRE(v.index() == 1);
			REQUIRE(plg::get<1>(v) == x);
		}
		{
			using V = plg::variant<const int, volatile int, int>;
			int x = 42;
			V v(plg::in_place_index<2>, x);
			REQUIRE(v.index() == 2);
			REQUIRE(plg::get<2>(v) == x);
		}
	}
	SECTION("explicit variant(in_place_index_t<I>, Args&&...) > sfinae") {
		{
			using V = plg::variant<int>;
			static_assert(std::is_constructible<V, plg::in_place_index_t<0>, int>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<0>, int>());
		}
		{
			using V = plg::variant<int, long, long long>;
			static_assert(std::is_constructible<V, plg::in_place_index_t<1>, int>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<1>, int>());
		}
		{
			using V = plg::variant<int, long, int*>;
			static_assert(std::is_constructible<V, plg::in_place_index_t<2>, int*>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<2>, int*>());
		}
		{// args not convertible to type
			using V = plg::variant<int, long, int*>;
			static_assert(!std::is_constructible<V, plg::in_place_index_t<0>, int*>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<0>, int*>());
		}
		{// index not in variant
			using V = plg::variant<int, long, int*>;
			static_assert(!std::is_constructible<V, plg::in_place_index_t<3>, int>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<3>, int>());
		}
	}
}
