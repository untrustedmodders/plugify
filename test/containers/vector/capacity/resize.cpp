#include <catch_amalgamated.hpp>

#include <app/counter.hpp>
#include <plg/vector.hpp>

#include <stdexcept>

TEST_CASE("vector capacity > resize", "[vector]") {
	SECTION("resize") {
		Counter counts;
		auto a = plg::vector<Counter::Obj>();
		a.resize(100);
		REQUIRE(a.size() == 100);
		for (auto const& x: a) {
			REQUIRE(x == Counter::Obj());
		}

		a.clear();
		for (size_t i = 0; i < 20; ++i) {
			a.emplace_back(i + 100, counts);
		}

		a.resize(10);
		REQUIRE(a.size() == 10);
		for (size_t i = 0; i < a.size(); ++i) {
			REQUIRE(a[i].get() == i + 100);
		}

		a.resize(3);
		REQUIRE(a.size() == 3);
		for (size_t i = 0; i < a.size(); ++i) {
			REQUIRE(a[i].get() == i + 100);
		}

		a.resize(0);
		REQUIRE(a.empty());
		a.clear();
	}
}
