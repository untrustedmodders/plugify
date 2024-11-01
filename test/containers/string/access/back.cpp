#include <catch_amalgamated.hpp>

#include <plugify/string.hpp>

TEST_CASE("string access > back", "[string]") {

	 SECTION("reference back() noexcept") {
		  plg::string a{"small", 5};
		  REQUIRE(a.back() == 'l');
		  a.back() = 't';
		  REQUIRE(a.back() == 't');

		  plg::string b{"longlonglonglonglonglonglonglong", 32};
		  REQUIRE(b.back() == 'g');
		  b.back() = 't';
		  REQUIRE(b.back() == 't');
	 }

	 SECTION("const_reference back() const noexcept") {
		  plg::string const a{"small", 5};
		  REQUIRE(a.back() == 'l');

		  plg::string const b{"longlonglonglonglonglonglonglong", 32};
		  REQUIRE(b.back() == 'g');
	 }

}
