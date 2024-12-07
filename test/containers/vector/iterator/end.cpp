#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

#include <string_view>

using namespace std::literals;

TEST_CASE("vector iterator > end", "[vector]") {
	SECTION("end") {
		plg::vector<int64_t> v;
		const auto& c = v;
		
		REQUIRE(v.end() == v.end());
		REQUIRE(v.end() == v.cend());
		REQUIRE(v.end() == v.begin());
		REQUIRE(v.end() == v.cbegin());

		REQUIRE(v.end() == c.end());
		REQUIRE(v.end() == c.cend());
		REQUIRE(v.end() == c.begin());
		REQUIRE(v.end() == c.cbegin());

		REQUIRE(c.end() == v.end());
		REQUIRE(c.end() == v.cend());
		REQUIRE(c.end() == v.begin());
		REQUIRE(c.end() == v.cbegin());

		REQUIRE(c.end() == c.end());
		REQUIRE(c.end() == c.cend());
		REQUIRE(c.end() == c.begin());
		REQUIRE(c.end() == c.cbegin());

		v.push_back(1);

		REQUIRE(1 == *std::prev(v.end()));
		REQUIRE(1 == *std::prev(c.end()));

		REQUIRE(v.end() == v.end());
		REQUIRE(v.end() == v.cend());
		REQUIRE(v.end() != v.begin());
		REQUIRE(v.end() != v.cbegin());

		REQUIRE(std::prev(v.end()) == v.begin());
		REQUIRE(std::prev(v.end()) == v.cbegin());

		REQUIRE(v.end() == c.end());
		REQUIRE(v.end() == c.cend());
		REQUIRE(v.end() != c.begin());
		REQUIRE(v.end() != c.cbegin());

		REQUIRE(std::prev(v.end()) == c.begin());
		REQUIRE(std::prev(v.end()) == c.cbegin());

		REQUIRE(c.end() == v.end());
		REQUIRE(c.end() == v.cend());
		REQUIRE(c.end() != v.begin());
		REQUIRE(c.end() != v.cbegin());

		REQUIRE(std::prev(c.end()) == v.begin());
		REQUIRE(std::prev(c.end()) == v.cbegin());

		REQUIRE(c.end() == c.end());
		REQUIRE(c.end() == c.cend());
		REQUIRE(c.end() != c.begin());
		REQUIRE(c.end() != c.cbegin());

		REQUIRE(std::prev(c.end()) == c.begin());
		REQUIRE(std::prev(c.end()) == c.cbegin());

		v.push_back(2);

		REQUIRE(2 == *std::prev(v.end()));
		REQUIRE(2 == *std::prev(c.end()));

		REQUIRE(v.end() == v.end());
		REQUIRE(v.end() == v.cend());
		REQUIRE(v.end() != v.begin());
		REQUIRE(v.end() != v.cbegin());

		REQUIRE(std::prev(v.end(), 2) == v.begin());
		REQUIRE(std::prev(v.end(), 2) == v.cbegin());

		REQUIRE(v.end() == c.end());
		REQUIRE(v.end() == c.cend());
		REQUIRE(v.end() != c.begin());
		REQUIRE(v.end() != c.cbegin());

		REQUIRE(std::prev(v.end(), 2) == c.begin());
		REQUIRE(std::prev(v.end(), 2) == c.cbegin());

		REQUIRE(c.end() == v.end());
		REQUIRE(c.end() == v.cend());
		REQUIRE(c.end() != v.begin());
		REQUIRE(c.end() != v.cbegin());

		REQUIRE(std::prev(c.end(), 2) == v.begin());
		REQUIRE(std::prev(c.end(), 2) == v.cbegin());

		REQUIRE(c.end() == c.end());
		REQUIRE(c.end() == c.cend());
		REQUIRE(c.end() != c.begin());
		REQUIRE(c.end() != c.cbegin());

		REQUIRE(std::prev(c.end(), 2) == c.begin());
		REQUIRE(std::prev(c.end(), 2) == c.cbegin());

		v.clear();

		REQUIRE(v.end() == v.end());
		REQUIRE(v.end() == v.cend());
		REQUIRE(v.end() == v.begin());
		REQUIRE(v.end() == v.cbegin());

		REQUIRE(v.end() == c.end());
		REQUIRE(v.end() == c.cend());
		REQUIRE(v.end() == c.begin());
		REQUIRE(v.end() == c.cbegin());

		REQUIRE(c.end() == v.end());
		REQUIRE(c.end() == v.cend());
		REQUIRE(c.end() == v.begin());
		REQUIRE(c.end() == v.cbegin());

		REQUIRE(c.end() == c.end());
		REQUIRE(c.end() == c.cend());
		REQUIRE(c.end() == c.begin());
		REQUIRE(c.end() == c.cbegin());
	}

	SECTION("cend") {
		plg::vector<int> v;
		
		REQUIRE(v.cend() == v.end());

		v.push_back(1);

		REQUIRE(1 == *std::prev(v.cend()));
		REQUIRE(v.cend() == v.end());

		v.push_back(2);

		REQUIRE(2 == *std::prev(v.cend()));
		REQUIRE(v.cend() == v.end());

		v.insert(v.begin() + 2, 3);

		REQUIRE(3 == *std::prev(v.cend()));
		REQUIRE(v.cend() == v.end());
	}
}
