#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

#include <app/counter.hpp>

TEST_CASE("vector operator > access > operator[]", "[vector]") {
	SECTION("idx]") {
		plg::vector<int> v { 1, 2, 3 };
		
		REQUIRE(v[0] == v.at(0));
		REQUIRE(v[0] == *v.begin());
		REQUIRE(v[0] == *v.data());
		REQUIRE(v[0] == v.front());

		REQUIRE(v[1] == v.at(1));
		REQUIRE(v[1] == *std::next(v.begin()));
		REQUIRE(v[1] == *std::next(v.data()));
		REQUIRE(v[1] != v.front());

		REQUIRE(v[v.size() - 1] == v.at(v.size() - 1));
		REQUIRE(v[v.size() - 1] == *std::next(v.begin(), static_cast<std::ptrdiff_t>(v.size() - 1)));
		REQUIRE(v[v.size() - 1] == *std::next(v.data(), static_cast<std::ptrdiff_t>(v.size() - 1)));
		REQUIRE(v[v.size() - 1] != v.front());
		REQUIRE(v[v.size() - 1] == *std::prev(v.end()));
	}
}