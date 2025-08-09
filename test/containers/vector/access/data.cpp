#include <catch_amalgamated.hpp>

#include <plg/vector.hpp>

TEST_CASE("vector access > data", "[vector]") {

	SECTION("data") {
		{
			plg::vector<char> const a{'s', 'm', 'a', 'l', 'l'};
			REQUIRE(a.data()[0] == 's');
			REQUIRE(a.data()[4] == 'l');
		}
		{
			plg::vector<int> b;

			CHECK(b.empty());
			CHECK(b.data() == b.begin());

			b.push_back(1);
			CHECK(b.data() == b.begin());
			CHECK(1 == *b.data());

			b.push_back(2);
			b.push_back(3);
			b.push_back(4);
			b.push_back(5);
			CHECK(b.data() == b.begin());
			CHECK(1 == *b.data());
		}
	}
}
