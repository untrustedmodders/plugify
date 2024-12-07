#include <catch_amalgamated.hpp>

#include <app/counter.hpp>
#include <plugify/vector.hpp>

#include <stdexcept>

TEST_CASE("vector capacity > max_size", "[vector]") {
	SECTION("max_size") {
		plg::vector<int> a;
		REQUIRE(a.max_size() == 0x3fffffffffffffff);

		plg::vector<double> b(100, 20.0);
		REQUIRE(b.max_size() == 0x1fffffffffffffff);
	}
}