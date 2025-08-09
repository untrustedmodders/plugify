#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

#include "app/test_convertible.hpp"

namespace {
	struct InitList {
		std::size_t size;
		constexpr InitList(std::initializer_list<int> il) : size(il.size()) {}
	};

	struct InitListArg {
		std::size_t size;
		int value;
		constexpr InitListArg(std::initializer_list<int> il, int v)
			: size(il.size()), value(v) {}
	};
} // namespace

TEST_CASE("variant index list constructor", "[variant]") {
	SECTION("explicit variant(in_place_index_t<I>, initializer_list<Up>, Args&&...) > basic") {
		{
			constexpr plg::variant<InitList, InitListArg, InitList> v(plg::in_place_index<0>, {1, 2, 3});
			static_assert(v.index() == 0);
			static_assert(plg::get<0>(v).size == 3);
		}
		{
			constexpr plg::variant<InitList, InitListArg, InitList> v(
					plg::in_place_index<2>, {1, 2, 3});
			static_assert(v.index() == 2);
			static_assert(plg::get<2>(v).size == 3);
		}
		{
			constexpr plg::variant<InitList, InitListArg, InitListArg> v(
					plg::in_place_index<1>, {1, 2, 3, 4}, 42);
			static_assert(v.index() == 1);
			static_assert(plg::get<1>(v).size == 4);
			static_assert(plg::get<1>(v).value == 42);
		}
	}
	SECTION("explicit variant(in_place_index_t<I>, initializer_list<Up>, Args&&...) > sfinae") {
		using IL = std::initializer_list<int>;
		{// just init list
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(std::is_constructible<V, plg::in_place_index_t<0>, IL>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<0>, IL>());
		}
		{// too many arguments
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(!std::is_constructible<V, plg::in_place_index_t<0>, IL, int>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<0>, IL, int>());
		}
		{// too few arguments
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(!std::is_constructible<V, plg::in_place_index_t<1>, IL>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<1>, IL>());
		}
		{// init list and arguments
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(std::is_constructible<V, plg::in_place_index_t<1>, IL, int>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<1>, IL, int>());
		}
		{// not constructible from arguments
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(!std::is_constructible<V, plg::in_place_index_t<2>, IL>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<2>, IL>());
		}
		{// index not in variant
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(!std::is_constructible<V, plg::in_place_index_t<3>, IL>::value);
			static_assert(!test_convertible<V, plg::in_place_index_t<3>, IL>());
		}
	}
}
