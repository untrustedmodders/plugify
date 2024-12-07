#include <catch_amalgamated.hpp>

#include <plugify/string.hpp>

TEST_CASE("string iterator > operator > increment", "[string]") {

	SECTION("operator++() noexcept") {
		plg::string c(1, '\0');
		typename plg::string::iterator i = c.begin();
		++i;
		REQUIRE(i == c.end());
	}

}
