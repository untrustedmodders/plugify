#include <catch_amalgamated.hpp>

#include <cstddef>

#include <plg/string.hpp>

TEST_CASE("string capacity > max_size", "[string]") {

	 SECTION("size_type max_size() const noexcept") {
		  plg::string a;
		  REQUIRE(a.max_size() == 0x7fffffffffffffff);
	 }

}
