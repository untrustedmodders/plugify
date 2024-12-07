#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

#include <app/counter.hpp>

TEST_CASE("vector operator != comparison > n_equal", "[vector]") {
	SECTION("n_eq_0") {
		auto counts = Counter();
		INFO(counts);

		auto sv = plg::vector<Counter::Obj>();
		REQUIRE(sv.capacity() == 0);

		sv.emplace_back(123, counts);
		REQUIRE(sv.capacity() == 1);
		sv.emplace_back(124, counts);
		REQUIRE(sv.capacity() == 2);

		sv.emplace_back(125, counts);
		REQUIRE(sv.capacity() == 4);
		REQUIRE(sv.size() == 3);

		sv.clear();
		REQUIRE(sv.empty());
		REQUIRE(sv.capacity() == 4);

		sv.shrink_to_fit();
		REQUIRE(sv.capacity() == 0);
	}
}
