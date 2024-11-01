#include <catch_amalgamated.hpp>

#include <app/counter.hpp>
#include <plugify/vector.hpp>

#include <stdexcept>

TEST_CASE("vector capacity > max_size", "[vector]") {
	SECTION("max_size") {
		const size_t max_size = 0x7fffffffffffffff;

		plg::vector<int> a;
		REQUIRE(max_size == a.max_size());

		plg::vector<double> b(100, 20.0);
		REQUIRE(max_size == b.max_size());
	}
}