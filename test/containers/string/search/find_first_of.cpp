#include <catch_amalgamated.hpp>

#include <plg/string.hpp>

TEST_CASE("string search > find_first_of", "[string]") {

	 SECTION("size_type find_first_of(string const& str, size_type pos = 0) const noexcept") {
		  using csize_type = plg::string::size_type;
		  csize_type npos = plg::string::npos;
		  csize_type csz01;

		  const char str_lit01[] = "mave";
		  const plg::string str01("mavericks, santa cruz");
		  plg::string str02(str_lit01);
		  plg::string str03("s, s");
		  plg::string str04;

		  plg::string str05("xena rulez");
		  csz01 = str01.find_first_of(str01);
		  REQUIRE(csz01 == 0);
		  csz01 = str01.find_first_of(str01, 4);
		  REQUIRE(csz01 == 4);
		  csz01 = str01.find_first_of(str02, 0);
		  REQUIRE(csz01 == 0);
		  csz01 = str01.find_first_of(str02, 3);
		  REQUIRE(csz01 == 3);
		  csz01 = str01.find_first_of(str03, 0);
		  REQUIRE(csz01 == 8);
		  csz01 = str01.find_first_of(str03, 3);
		  REQUIRE(csz01 == 8);
		  csz01 = str01.find_first_of(str03, 12);
		  REQUIRE(csz01 == 16);
		  csz01 = str01.find_first_of(str05, 0);
		  REQUIRE(csz01 == 1);
		  csz01 = str01.find_first_of(str05, 4);
		  REQUIRE(csz01 == 4);

		  csz01 = str01.find_first_of(str04, 0);
		  REQUIRE(csz01 == npos);
		  csz01 = str01.find_first_of(str04, 5);
		  REQUIRE(csz01 == npos);
	 }

	 SECTION("size_type find_first_of(const_pointer s, size_type pos, size_type count) const") {
		  using csize_type = plg::string::size_type;
		  csize_type npos = plg::string::npos;
		  csize_type csz01;

		  const char str_lit01[] = "mave";
		  const plg::string str01("mavericks, santa cruz");

		  csz01 = str01.find_first_of(str_lit01, 0, 3);
		  REQUIRE(csz01 == 0);
		  csz01 = str01.find_first_of(str_lit01, 3, 0);
		  REQUIRE(csz01 == npos);
	 }

	 SECTION("size_type find_first_of(const_pointer s, size_type pos = 0) const") {
		  using csize_type = plg::string::size_type;
		  csize_type csz01;

		  const char str_lit01[] = "mave";
		  const plg::string str01("mavericks, santa cruz");

		  csz01 = str01.find_first_of(str_lit01);
		  REQUIRE(csz01 == 0);
		  csz01 = str01.find_first_of(str_lit01, 3);
		  REQUIRE(csz01 == 3);
	 }

	 SECTION("size_type find_first_of(value_type ch, size_type pos = 0) const noexcept") {
		  using csize_type = plg::string::size_type;
		  csize_type csz01, csz02;

		  const plg::string str01("mavericks, santa cruz");

		  csz01 = str01.find_first_of('z');
		  csz02 = str01.size() - 1;
		  REQUIRE(csz01 == csz02);
	 }

	 SECTION("size_type find_first_of(T const& t, size_type pos = 0) const noexcept") {
		  std::string_view str1("bar");
		  plg::string str2("foobar");

		  str2 = "foobarxyz";
		  str1 = "zyx";
		  auto x = str2.find_first_of(str1);
		  REQUIRE(x == 6);

		  str2 = "foobarxyzfooxyz";
		  x = str2.find_first_of(str1);
		  REQUIRE(x == 6);
		  x = str2.find_first_of(str1, 9);
		  REQUIRE(x == 12);
	 }

}
