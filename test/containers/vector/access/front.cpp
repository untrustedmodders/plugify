#include <catch_amalgamated.hpp>

#include <plg/vector.hpp>

TEST_CASE("vector access > front", "[vector]") {

	SECTION("front") {
		{
			plg::vector<char> a{'s', 'm', 'a', 'l', 'l'};
			REQUIRE(a.front() == 's');
			a.front() = 't';
			REQUIRE(a.front() == 't');
		}
		{
			plg::vector<int> b{ 2 };

			REQUIRE(b.front() == 2);
			REQUIRE(b.front() == b[0]);

			b.push_back(3);

			REQUIRE(b.front() == 2);
			REQUIRE(b.front() == b[0]);

			b.insert(b.begin(), 1);

			REQUIRE(b.front() == 1);
			REQUIRE(b.front() == b[0]);
		}
	}
}
