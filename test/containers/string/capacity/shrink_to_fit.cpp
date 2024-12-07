#include <catch_amalgamated.hpp>

#include <plugifystring.hpp>

TEST_CASE("string capacity > shrink_to_fit", "[string]") {

	 SECTION("void shrink_to_fit()") {
		  plg::string a;
		  a.shrink_to_fit();
		  REQUIRE(a.capacity() == 23);
		  REQUIRE(a.size() == 0);

		  plg::string b{"toto", 4};
		  b.shrink_to_fit();
		  REQUIRE(b.capacity() == 23);
		  REQUIRE(b.size() == 4);
		  REQUIRE(b[0] == 't');
		  REQUIRE(b[3] == 'o');

		  plg::string c{"toto", 4};
		  c.reserve(32);
		  c.shrink_to_fit();
		  REQUIRE(c.capacity() == 4);
		  REQUIRE(c.size() == 4);
		  REQUIRE(c[0] == 't');
		  REQUIRE(c[3] == 'o');

		  plg::string d{"totototototototototototototototo", 32};
		  d.reserve(65);
		  d.shrink_to_fit();
		  REQUIRE(d.capacity() == 32);
		  REQUIRE(d.size() == 32);
		  REQUIRE(d[0] == 't');
		  REQUIRE(d[31] == 'o');
	 }

}
