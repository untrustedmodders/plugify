#include <catch_amalgamated.hpp>

#include <plugify/variant.hpp>

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

TEST_CASE("variant type constructor", "[variant]") {
	SECTION("explicit variant(in_place_type_t<Tp>, initializer_list<Up>, Args&&...) > basic") {
		using IL = std::initializer_list<int>;
		{// just init list
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(std::is_constructible<V, plg::in_place_type_t<InitList>, IL>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<InitList>, IL>());
		}
		{// too many arguments
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(!std::is_constructible<V, plg::in_place_type_t<InitList>, IL, int>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<InitList>, IL, int>());
		}
		{// too few arguments
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(!std::is_constructible<V, plg::in_place_type_t<InitListArg>, IL>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<InitListArg>, IL>());
		}
		{// init list and arguments
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(std::is_constructible<V, plg::in_place_type_t<InitListArg>, IL, int>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<InitListArg>, IL, int>());
		}
		{// not constructible from arguments
			using V = plg::variant<InitList, InitListArg, int>;
			static_assert(!std::is_constructible<V, plg::in_place_type_t<int>, IL>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<int>, IL>());
		}
		{// duplicate types in variant
			using V = plg::variant<InitListArg, InitListArg, int>;
			static_assert(!std::is_constructible<V, plg::in_place_type_t<InitListArg>, IL, int>::value);
			static_assert(!test_convertible<V, plg::in_place_type_t<InitListArg>, IL, int>());
		}
	}
	SECTION("explicit variant(in_place_type_t<Tp>, initializer_list<Up>, Args&&...) > sfinae") {
		{
			constexpr plg::variant<InitList, InitListArg> v(plg::in_place_type<InitList>, {1, 2, 3});
			static_assert(v.index() == 0);
			static_assert(plg::get<0>(v).size == 3);
		}
		{
			constexpr plg::variant<InitList, InitListArg> v(plg::in_place_type<InitListArg>, {1, 2, 3, 4}, 42);
			static_assert(v.index() == 1);
			static_assert(plg::get<1>(v).size == 4);
			static_assert(plg::get<1>(v).value == 42);
		}
	}
}
