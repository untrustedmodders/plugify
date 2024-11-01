#include <catch_amalgamated.hpp>

#include <app/counter.hpp>
#include <plugify/vector.hpp>

#include <stdexcept>

TEST_CASE("vector capacity > max_size", "[vector]") {
	SECTION("max_size") {
		plg::vector<int> v;
		REQUIRE(0x7fffffffffffffff == v.max_size());

		plg::vector<double> w;
		REQUIRE(0x7fffffffffffffff == w.max_size());
	}
}