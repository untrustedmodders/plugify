#include <catch_amalgamated.hpp>

#include <plugify/string.hpp>

TEST_CASE("string iterator > operator > decrement", "[string]") {

	SECTION("operator--() noexcept") {
		plg::string c(1, '\0');
		typename plg::string::iterator i = c.end();
		--i;
		REQUIRE(i == c.begin());
	}

}
