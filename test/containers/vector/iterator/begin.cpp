#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

#include <string_view>

using namespace std::literals;

TEST_CASE("vector iterator > begin", "[vector]") {
	SECTION("begin") {
		plg::vector<int64_t> v;
		const auto& c = v;
		
		REQUIRE(v.begin() == v.begin());
		REQUIRE(v.begin() == v.cbegin());
		REQUIRE(v.begin() == v.end());
		REQUIRE(v.begin() == v.cend());

		REQUIRE(v.begin() == c.begin());
		REQUIRE(v.begin() == c.cbegin());
		REQUIRE(v.begin() == c.end());
		REQUIRE(v.begin() == c.cend());

		REQUIRE(c.begin() == v.begin());
		REQUIRE(c.begin() == v.cbegin());
		REQUIRE(c.begin() == v.end());
		REQUIRE(c.begin() == v.cend());

		REQUIRE(c.begin() == c.begin());
		REQUIRE(c.begin() == c.cbegin());
		REQUIRE(c.begin() == c.end());
		REQUIRE(c.begin() == c.cend());

		v.push_back(1);

		REQUIRE(1 == *v.begin());
		REQUIRE(1 == *c.begin());

		REQUIRE(v.begin() == v.begin());
		REQUIRE(v.begin() == v.cbegin());
		REQUIRE(v.begin() != v.end());
		REQUIRE(v.begin() != v.cend());

		REQUIRE(std::next(v.begin()) == v.end());
		REQUIRE(std::next(v.begin()) == v.cend());

		REQUIRE(v.begin() == c.begin());
		REQUIRE(v.begin() == c.cbegin());
		REQUIRE(v.begin() != c.end());
		REQUIRE(v.begin() != c.cend());

		REQUIRE(std::next(v.begin()) == c.end());
		REQUIRE(std::next(v.begin()) == c.cend());

		REQUIRE(c.begin() == v.begin());
		REQUIRE(c.begin() == v.cbegin());
		REQUIRE(c.begin() != v.end());
		REQUIRE(c.begin() != v.cend());

		REQUIRE(std::next(c.begin()) == v.end());
		REQUIRE(std::next(c.begin()) == v.cend());

		REQUIRE(c.begin() == c.begin());
		REQUIRE(c.begin() == c.cbegin());
		REQUIRE(c.begin() != c.end());
		REQUIRE(c.begin() != c.cend());

		REQUIRE(std::next(c.begin()) == c.end());
		REQUIRE(std::next(c.begin()) == c.cend());

		v.push_back(2);

		REQUIRE(1 == *v.begin());
		REQUIRE(1 == *c.begin());

		REQUIRE(v.begin() == v.begin());
		REQUIRE(v.begin() == v.cbegin());
		REQUIRE(v.begin() != v.end());
		REQUIRE(v.begin() != v.cend());

		REQUIRE(std::next(v.begin(), 2) == v.end());
		REQUIRE(std::next(v.begin(), 2) == v.cend());

		REQUIRE(v.begin() == c.begin());
		REQUIRE(v.begin() == c.cbegin());
		REQUIRE(v.begin() != c.end());
		REQUIRE(v.begin() != c.cend());

		REQUIRE(std::next(v.begin(), 2) == c.end());
		REQUIRE(std::next(v.begin(), 2) == c.cend());

		REQUIRE(c.begin() == v.begin());
		REQUIRE(c.begin() == v.cbegin());
		REQUIRE(c.begin() != v.end());
		REQUIRE(c.begin() != v.cend());

		REQUIRE(std::next(c.begin(), 2) == v.end());
		REQUIRE(std::next(c.begin(), 2) == v.cend());

		REQUIRE(c.begin() == c.begin());
		REQUIRE(c.begin() == c.cbegin());
		REQUIRE(c.begin() != c.end());
		REQUIRE(c.begin() != c.cend());

		REQUIRE(std::next(c.begin(), 2) == c.end());
		REQUIRE(std::next(c.begin(), 2) == c.cend());

		v.clear();

		REQUIRE(v.begin() == v.begin());
		REQUIRE(v.begin() == v.cbegin());
		REQUIRE(v.begin() == v.end());
		REQUIRE(v.begin() == v.cend());

		REQUIRE(v.begin() == c.begin());
		REQUIRE(v.begin() == c.cbegin());
		REQUIRE(v.begin() == c.end());
		REQUIRE(v.begin() == c.cend());

		REQUIRE(c.begin() == v.begin());
		REQUIRE(c.begin() == v.cbegin());
		REQUIRE(c.begin() == v.end());
		REQUIRE(c.begin() == v.cend());

		REQUIRE(c.begin() == c.begin());
		REQUIRE(c.begin() == c.cbegin());
		REQUIRE(c.begin() == c.end());
		REQUIRE(c.begin() == c.cend());
	}

	SECTION("cbegin") {
		plg::vector<int> v;
		
		REQUIRE(v.cbegin() == v.begin());

		v.push_back(1);

		REQUIRE(1 == *v.cbegin());
		REQUIRE(v.cbegin() == v.begin());

		v.push_back(2);

		REQUIRE(1 == *v.cbegin());
		REQUIRE(v.cbegin() == v.begin());

		v.insert(v.begin(), 3);

		REQUIRE(3 == *v.cbegin());
		REQUIRE(v.cbegin() == v.begin());
	}
}
