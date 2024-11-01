#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

TEST_CASE("vector access > back", "[vector]") {

	SECTION("back") {
		{
			plg::vector<char> a{'s', 'm', 'a', 'l', 'l'};
			REQUIRE(a.back() == 'l');
			a.back() = 't';
			REQUIRE(a.back() == 't');
		}
		{
			plg::vector<int> b{ 2 };

			REQUIRE(b.back() == 2);
			REQUIRE(b.back() == b[b.size() - 1]);

			b.push_back(3);

			REQUIRE(b.back() == 3);
			REQUIRE(b.back() == b[b.size() - 1]);

			b.insert(b.begin(), 1);

			REQUIRE(b.back() == 3);
			REQUIRE(b.back() == b[b.size() - 1]);
		}
	}
}
