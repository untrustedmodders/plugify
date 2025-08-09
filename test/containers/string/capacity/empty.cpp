#include <catch_amalgamated.hpp>

#include <plg/string.hpp>

TEST_CASE("string capacity > empty", "[string]") {

	 SECTION("[[nodiscard]] bool empty() const noexcept") {
		  plg::string a;
		  REQUIRE(a.empty());

		  plg::string b{"toto", 4};
		  REQUIRE_FALSE(b.empty());
	 }

}
