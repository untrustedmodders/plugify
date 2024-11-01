#include <catch_amalgamated.hpp>

#include <cstring>

#include <plugify/string.hpp>

TEST_CASE("string capacity > reserve", "[string]") {

	 SECTION("void reserve(size_type new_cap)") {
		  plg::string a;
		  a.reserve(1);
		  REQUIRE(a.capacity() == 23);
		  a.reserve(23);
		  REQUIRE(a.capacity() == 23);
		  a.reserve(24);
		  REQUIRE(a.capacity() == 24);
		  a.reserve(25);
		  REQUIRE(a.capacity() == 25);

		  plg::string b{"toto", 4};
		  b.reserve(5);
		  REQUIRE(b.capacity() == 23);
		  REQUIRE(std::memcmp(b.data(), "toto", 4) == 0);
		  b.reserve(23);
		  REQUIRE(b.capacity() == 23);
		  REQUIRE(std::memcmp(b.data(), "toto", 4) == 0);
		  b.reserve(24);
		  REQUIRE(b.capacity() == 24);
		  REQUIRE(std::memcmp(b.data(), "toto", 4) == 0);
		  b.reserve(25);
		  REQUIRE(b.capacity() == 25);
		  REQUIRE(std::memcmp(b.data(), "toto", 4) == 0);
	 }

}
