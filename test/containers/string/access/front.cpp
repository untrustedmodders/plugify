#include <catch_amalgamated.hpp>

#include <plugifystring.hpp>

TEST_CASE("string access > front", "[string]") {

	 SECTION("reference front() noexcept") {
		  plg::string a{"small", 5};
		  REQUIRE(a.front() == 's');
		  a.front() = 't';
		  REQUIRE(a.front() == 't');

		  plg::string b{"longlonglonglonglonglonglonglong", 32};
		  REQUIRE(b.front() == 'l');
		  b.front() = 't';
		  REQUIRE(b.front() == 't');
	 }

	 SECTION("const_reference front() const noexcept") {
		  plg::string const a{"small", 5};
		  REQUIRE(a.front() == 's');

		  plg::string const b{"longlonglonglonglonglonglonglong", 32};
		  REQUIRE(b.front() == 'l');
	 }

}
