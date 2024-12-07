#include <catch_amalgamated.hpp>

#include <app/counter.hpp>
#include <app/vector_tester.hpp>
#include <plugify/vector.hpp>

#include <vector>

TEST_CASE("vector operation > clear", "[vector]") {
	SECTION("clear") {
		plg::vector<float> v;

		v.clear();
		REQUIRE(v.empty());

		v.push_back(1);
		REQUIRE(! v.empty());

		v.clear();
		REQUIRE(v.empty());

		v.insert(v.end(), 2, 2.0f);
		v.clear();

		REQUIRE(v.empty());
	}
}