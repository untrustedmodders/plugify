#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

namespace {
	struct NonTDtor {
		static int count;
		NonTDtor() = default;
		~NonTDtor() { ++count; }
	};
	int NonTDtor::count = 0;
	static_assert(!std::is_trivially_destructible<NonTDtor>::value);

	struct NonTDtor1 {
		static int count;
		NonTDtor1() = default;
		~NonTDtor1() { ++count; }
	};
	int NonTDtor1::count = 0;
	static_assert(!std::is_trivially_destructible<NonTDtor1>::value);

	struct TDtor {
		TDtor(const TDtor&) {}// non-trivial copy
		~TDtor() = default;
	};
	static_assert(!std::is_trivially_copy_constructible<TDtor>::value);
	static_assert(std::is_trivially_destructible<TDtor>::value);
}

TEST_CASE("variant destructor", "[variant]") {
	SECTION("~variant()") {
		{
			using V = plg::variant<int, long, TDtor>;
			static_assert(std::is_trivially_destructible<V>::value);
		}
		{
			using V = plg::variant<NonTDtor, int, NonTDtor1>;
			static_assert(!std::is_trivially_destructible<V>::value);
			{
				V v(plg::in_place_index<0>);
				REQUIRE(NonTDtor::count == 0);
				REQUIRE(NonTDtor1::count == 0);
			}
			REQUIRE(NonTDtor::count == 1);
			REQUIRE(NonTDtor1::count == 0);
			NonTDtor::count = 0;
			{ V v(plg::in_place_index<1>); }
			REQUIRE(NonTDtor::count == 0);
			REQUIRE(NonTDtor1::count == 0);
			{
				V v(plg::in_place_index<2>);
				REQUIRE(NonTDtor::count == 0);
				REQUIRE(NonTDtor1::count == 0);
			}
			REQUIRE(NonTDtor::count == 0);
			REQUIRE(NonTDtor1::count == 1);
		}
	}
}
