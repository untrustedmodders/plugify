#include <catch_amalgamated.hpp>

#include <plugifyvector.hpp>

#include <stdexcept>

TEST_CASE("vector access > at", "[vector]") {

	SECTION("at") {
		{
			auto sv = plg::vector<std::string>();
			auto const& svConst = sv;
			REQUIRE_THROWS_AS(sv.at(0), std::out_of_range);
			REQUIRE_THROWS_AS(svConst.at(0), std::out_of_range);

			sv.push_back("hello");
			REQUIRE(sv.at(0) == "hello");
			REQUIRE(svConst.at(0) == "hello");
			REQUIRE_THROWS_AS(sv.at(1), std::out_of_range);
			REQUIRE_THROWS_AS(svConst.at(1), std::out_of_range);

			sv.resize(100);
			REQUIRE(sv.size() == 100);
			REQUIRE(sv.at(99) == "");
			REQUIRE(svConst.at(99) == "");
			REQUIRE_THROWS_AS(sv.at(100), std::out_of_range);
			REQUIRE_THROWS_AS(svConst.at(100), std::out_of_range);
		}
		{
			plg::vector<int> v { 1, 2, 3 };
			REQUIRE(v.at (0) == v[0]);
			REQUIRE(v.at (0) == v.front());
			REQUIRE(v.at (1) == v[1]);
			REQUIRE(v.at (v.size () - 1) == v[v.size () - 1]);
			REQUIRE(v.at (v.size () - 1) == v.back());
		}
	}
}
