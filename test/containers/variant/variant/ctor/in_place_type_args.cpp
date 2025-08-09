#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

#include "app/test_convertible.hpp"

TEST_CASE("variant args constructor", "[variant]") {
	SECTION("explicit variant(in_place_type_t<Tp>, Args&&...) > basic") {
		{
			using V = plg::variant<int>;
			static_assert(std::is_constructible<V, plg::in_place_type_t<int>, int>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<int>, int>());
		}
		{
			using V = plg::variant<int, long, long long>;
			static_assert(std::is_constructible<V, plg::in_place_type_t<long>, int>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<long>, int>());
		}
		{
			using V = plg::variant<int, long, int*>;
			static_assert(std::is_constructible<V, plg::in_place_type_t<int*>, int*>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<int*>, int*>());
		}
		{// duplicate type
			using V = plg::variant<int, long, int>;
			static_assert(!std::is_constructible<V, plg::in_place_type_t<int>, int>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<int>, int>());
		}
		{// args not convertible to type
			using V = plg::variant<int, long, int*>;
			static_assert(!std::is_constructible<V, plg::in_place_type_t<int>, int*>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<int>, int*>());
		}
		{// type not in variant
			using V = plg::variant<int, long, int*>;
			static_assert(!std::is_constructible<V, plg::in_place_type_t<long long>, int>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<long long>, int>());
		}
	}
	SECTION("explicit variant(in_place_type_t<Tp>, Args&&...)  > sfinae") {
		{
			constexpr plg::variant<int> v(plg::in_place_type<int>, 42);
			static_assert(v.index() == 0);
			static_assert(plg::get<0>(v) == 42);
		}
		{
			constexpr plg::variant<int, long> v(plg::in_place_type<long>, 42);
			static_assert(v.index() == 1);
			static_assert(plg::get<1>(v) == 42);
		}
		{
			constexpr plg::variant<int, const int, long> v(plg::in_place_type<const int>, 42);
			static_assert(v.index() == 1);
			static_assert(plg::get<1>(v) == 42);
		}
		{
			using V = plg::variant<const int, volatile int, int>;
			int x = 42;
			V v(plg::in_place_type<const int>, x);
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v) == x);
		}
		{
			using V = plg::variant<const int, volatile int, int>;
			int x = 42;
			V v(plg::in_place_type<volatile int>, x);
			REQUIRE(v.index() == 1);
			REQUIRE(plg::get<1>(v) == x);
		}
		{
			using V = plg::variant<const int, volatile int, int>;
			int x = 42;
			V v(plg::in_place_type<int>, x);
			REQUIRE(v.index() == 2);
			REQUIRE(plg::get<2>(v) == x);
		}
	}
}
