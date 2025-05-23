#include <catch_amalgamated.hpp>

#include <plugify/string.hpp>

TEST_CASE("string iterator > operator > add", "[string]") {

	SECTION("operator+=(const difference_type n) noexcept") {
		plg::string c(1, '\0');
		typename plg::string::iterator i = c.begin();
		i += 1;
		REQUIRE(i == c.end());
	}

}
