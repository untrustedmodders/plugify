#include <catch_amalgamated.hpp>

#include <plg/string.hpp>

TEST_CASE("string access > data", "[string]") {

	 SECTION("const_pointer data() const noexcept") {
		  plg::string const a{"small", 5};
		  REQUIRE(a.data()[0] == 's');
		  REQUIRE(a.data()[4] == 'l');
		  REQUIRE(a.data()[5] == '\0');

		  plg::string const b{"longlonglonglonglonglonglonglong", 32};
		  REQUIRE(b.data()[0] == 'l');
		  REQUIRE(b.data()[31] == 'g');
		  REQUIRE(b.data()[32] == '\0');

		  plg::string const c;
		  REQUIRE(c.data()[0] == '\0');
	 }

	 SECTION("pointer data() noexcept") {
		  plg::string a{"small", 5};
		  REQUIRE(a.data()[0] == 's');
		  REQUIRE(a.data()[4] == 'l');
		  REQUIRE(a.data()[5] == '\0');
		  a.data()[0] = 't';
		  REQUIRE(a.data()[0] == 't');
		  a.data()[4] = 't';
		  REQUIRE(a.data()[4] == 't');

		  plg::string b{"longlonglonglonglonglonglonglong", 32};
		  REQUIRE(b.data()[0] == 'l');
		  REQUIRE(b.data()[31] == 'g');
		  REQUIRE(b.data()[32] == '\0');
		  b.data()[0] = 't';
		  REQUIRE(b.data()[0] == 't');
		  b.data()[31] = 't';
		  REQUIRE(b.data()[31] == 't');

		  plg::string const c;
		  REQUIRE(c.data()[0] == '\0');
	 }

}
