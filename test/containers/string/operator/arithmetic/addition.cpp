#include <catch_amalgamated.hpp>

#include <plugifystring.hpp>

TEST_CASE("string operator > arithmetic > addition", "[string]") {

	 SECTION("string operator+(string const& lhs, string const& rhs)") {
		  plg::string const str_0("costa rica");
		  plg::string str_3("costa ricans");
		  plg::string str_4;
		  plg::string const str_6("ns");
		  str_4 = str_0 + str_6;
		  REQUIRE(str_4 == str_3);

		  plg::string const s1("small");
		  plg::string s1l("smallsmallsmallsmallsma");
		  plg::string const s1ll("longlonglonglonglonglong");

		  REQUIRE(s1 + s1 == plg::string("smallsmall"));
		  REQUIRE(s1 + s1l == plg::string("smallsmallsmallsmallsmallsma"));
		  REQUIRE(s1 + s1ll == plg::string("smalllonglonglonglonglonglong"));

		  REQUIRE(s1l + s1 == plg::string("smallsmallsmallsmallsmasmall"));
		  REQUIRE(s1l + s1l == plg::string("smallsmallsmallsmallsmasmallsmallsmallsmallsma"));
		  REQUIRE(s1l + s1ll == plg::string("smallsmallsmallsmallsmalonglonglonglonglonglong"));

		  REQUIRE(s1ll + s1 == plg::string("longlonglonglonglonglongsmall"));
		  REQUIRE(s1ll + s1l == plg::string("longlonglonglonglonglongsmallsmallsmallsmallsma"));
		  REQUIRE(s1ll + s1ll == plg::string("longlonglonglonglonglonglonglonglonglonglonglong"));
	 }

	 SECTION("string operator+(string const& lhs, string::const_pointer rhs)") {
		  plg::string str1, str2;
		  str1 = plg::string("8-chars_") + "8-chars_";
		  REQUIRE(str1 == "8-chars_8-chars_");
		  str2 = str1 + "7-chars";
		  REQUIRE(str1 == "8-chars_8-chars_");

		  plg::string const str_0("costa rica");
		  plg::string str_3("costa ricans");
		  plg::string str_4;
		  str_4 = str_0 + "ns";
		  REQUIRE(str_4 == str_3);

		  plg::string const s1("small");
		  REQUIRE(s1 + "small" == plg::string("smallsmall"));
		  REQUIRE(s1 + "smallsmallsmallsmallsma" == plg::string("smallsmallsmallsmallsmallsma"));
		  REQUIRE(s1 + "longlonglonglonglonglong" == plg::string("smalllonglonglonglonglonglong"));

		  plg::string s1l("smallsmallsmallsmallsma");
		  REQUIRE(s1l + "small" == plg::string("smallsmallsmallsmallsmasmall"));
		  REQUIRE(s1l + "smallsmallsmallsmallsma" == plg::string("smallsmallsmallsmallsmasmallsmallsmallsmallsma"));
		  REQUIRE(s1l + "longlonglonglonglonglong" == plg::string("smallsmallsmallsmallsmalonglonglonglonglonglong"));

		  plg::string const s1ll("longlonglonglonglonglong");
		  REQUIRE(s1ll + "small" == plg::string("longlonglonglonglonglongsmall"));
		  REQUIRE(s1ll + "smallsmallsmallsmallsma" == plg::string("longlonglonglonglonglongsmallsmallsmallsmallsma"));
		  REQUIRE(s1ll + "longlonglonglonglonglong" == plg::string("longlonglonglonglonglonglonglonglonglonglonglong"));
	 }

	 SECTION("string operator+(string const& lhs, string::value_type rhs)") {
		  plg::string const s1("small");
		  REQUIRE(s1 + 'a' == plg::string("smalla"));
		  plg::string s1l("smallsmallsmallsmallsma");
		  REQUIRE(s1l + 'a' == plg::string("smallsmallsmallsmallsmaa"));
		  plg::string const s1ll("longlonglonglonglonglong");
		  REQUIRE(s1ll + 'a' == plg::string("longlonglonglonglonglonga"));

		  plg::string str;
		  str = s1 + 'a';
		  str = str + 'a';
		  REQUIRE(str == plg::string("smallaa"));
	 }

	 SECTION("string operator+(string::const_pointer lhs, string const& rhs)") {
		  plg::string str_4;
		  plg::string const str_5(" marbella");
		  plg::string str_1("costa marbella");
		  str_4 = "costa" + str_5;
		  REQUIRE(str_4 == str_1);
	 }

	 SECTION("string operator+(string::value_type lhs, string const& rhs)") {
		  plg::string const s1("small");
		  REQUIRE('a' + s1 == plg::string("asmall"));
		  plg::string s1l("smallsmallsmallsmallsma");
		  REQUIRE('a' + s1l == plg::string("asmallsmallsmallsmallsma"));
		  plg::string const s1ll("longlonglonglonglonglong");
		  REQUIRE('a' + s1ll == plg::string("alonglonglonglonglonglong"));

		  plg::string str;
		  str = 'a' + s1;
		  str = 'a' + str;
		  REQUIRE(str == plg::string("aasmall"));
	 }

	 SECTION("string operator+(string&& lhs, string&& rhs)") {
		  REQUIRE(plg::string("small") + plg::string("small") == plg::string("smallsmall"));
		  REQUIRE(plg::string("smallsmallsmallsmallsma") + plg::string("small") == plg::string("smallsmallsmallsmallsmasmall"));
		  REQUIRE(plg::string("longlonglonglonglonglong") + plg::string("small") == plg::string("longlonglonglonglonglongsmall"));

		  REQUIRE(plg::string("small") + plg::string("smallsmallsmallsmallsma") == plg::string("smallsmallsmallsmallsmallsma"));
		  REQUIRE(plg::string("smallsmallsmallsmallsma") + plg::string("smallsmallsmallsmallsma") == plg::string("smallsmallsmallsmallsmasmallsmallsmallsmallsma"));
		  REQUIRE(plg::string("longlonglonglonglonglong") + plg::string("smallsmallsmallsmallsma") == plg::string("longlonglonglonglonglongsmallsmallsmallsmallsma"));

		  REQUIRE(plg::string("small") + plg::string("longlonglonglonglonglong") == plg::string("smalllonglonglonglonglonglong"));
		  REQUIRE(plg::string("smallsmallsmallsmallsma") + plg::string("longlonglonglonglonglong") == plg::string("smallsmallsmallsmallsmalonglonglonglonglonglong"));
		  REQUIRE(plg::string("longlonglonglonglonglong") + plg::string("longlonglonglonglonglong") == plg::string("longlonglonglonglonglonglonglonglonglonglonglong"));
	 }

	 SECTION("string operator+(string&& lhs, string const& rhs)") {
		  plg::string const s1("small");
		  REQUIRE(plg::string("small") + s1 == plg::string("smallsmall"));
		  REQUIRE(plg::string("smallsmallsmallsmallsma") + s1 == plg::string("smallsmallsmallsmallsmasmall"));
		  REQUIRE(plg::string("longlonglonglonglonglong") + s1 == plg::string("longlonglonglonglonglongsmall"));

		  plg::string s1l("smallsmallsmallsmallsma");
		  REQUIRE(plg::string("small") + s1l == plg::string("smallsmallsmallsmallsmallsma"));
		  REQUIRE(plg::string("smallsmallsmallsmallsma") + s1l == plg::string("smallsmallsmallsmallsmasmallsmallsmallsmallsma"));
		  REQUIRE(plg::string("longlonglonglonglonglong") + s1l == plg::string("longlonglonglonglonglongsmallsmallsmallsmallsma"));

		  plg::string const s1ll("longlonglonglonglonglong");
		  REQUIRE(plg::string("small") + s1ll == plg::string("smalllonglonglonglonglonglong"));
		  REQUIRE(plg::string("smallsmallsmallsmallsma") + s1ll == plg::string("smallsmallsmallsmallsmalonglonglonglonglonglong"));
		  REQUIRE(plg::string("longlonglonglonglonglong") + s1ll == plg::string("longlonglonglonglonglonglonglonglonglonglonglong"));
	 }

	 SECTION("string operator+(string&& lhs, string::const_pointer rhs)") {
		  REQUIRE(plg::string("small") + "small" == plg::string("smallsmall"));
		  REQUIRE(plg::string("smallsmallsmallsmallsma") + "small" == plg::string("smallsmallsmallsmallsmasmall"));
		  REQUIRE(plg::string("longlonglonglonglonglong") + "small" == plg::string("longlonglonglonglonglongsmall"));

		  REQUIRE(plg::string("small") + "smallsmallsmallsmallsma" == plg::string("smallsmallsmallsmallsmallsma"));
		  REQUIRE(plg::string("smallsmallsmallsmallsma") + "smallsmallsmallsmallsma" == plg::string("smallsmallsmallsmallsmasmallsmallsmallsmallsma"));
		  REQUIRE(plg::string("longlonglonglonglonglong") + "smallsmallsmallsmallsma" == plg::string("longlonglonglonglonglongsmallsmallsmallsmallsma"));

		  REQUIRE(plg::string("small") + "longlonglonglonglonglong" == plg::string("smalllonglonglonglonglonglong"));
		  REQUIRE(plg::string("smallsmallsmallsmallsma") + "longlonglonglonglonglong" == plg::string("smallsmallsmallsmallsmalonglonglonglonglonglong"));
		  REQUIRE(plg::string("longlonglonglonglonglong") + "longlonglonglonglonglong" == plg::string("longlonglonglonglonglonglonglonglonglonglonglong"));
	 }

	 SECTION("string operator+(string&& lhs, string::value_type rhs)") {
		  REQUIRE(plg::string("small") + 'a' == plg::string("smalla") );
		  REQUIRE(plg::string("longlonglonglonglonglon") + 'a' == plg::string("longlonglonglonglonglona"));
		  REQUIRE(plg::string("longlonglonglonglonglong") + 'a' == plg::string("longlonglonglonglonglonga"));
	 }

	 SECTION("string operator+(string const& lhs, string&& rhs)") {
		  plg::string const s1("small");
		  REQUIRE(s1 + plg::string("small") == plg::string("smallsmall"));
		  REQUIRE(s1 + plg::string("smallsmallsmallsmallsma") == plg::string("smallsmallsmallsmallsmallsma"));
		  REQUIRE(s1 + plg::string("longlonglonglonglonglong") == plg::string("smalllonglonglonglonglonglong"));

		  plg::string s1l("smallsmallsmallsmallsma");
		  REQUIRE(s1l + plg::string("small") == plg::string("smallsmallsmallsmallsmasmall"));
		  REQUIRE(s1l + plg::string("smallsmallsmallsmallsma") == plg::string("smallsmallsmallsmallsmasmallsmallsmallsmallsma"));
		  REQUIRE(s1l + plg::string("longlonglonglonglonglong") == plg::string("smallsmallsmallsmallsmalonglonglonglonglonglong"));

		  plg::string const s1ll("longlonglonglonglonglong");
		  REQUIRE(s1ll + plg::string("small") == plg::string("longlonglonglonglonglongsmall"));
		  REQUIRE(s1ll + plg::string("smallsmallsmallsmallsma") == plg::string("longlonglonglonglonglongsmallsmallsmallsmallsma"));
		  REQUIRE(s1ll + plg::string("longlonglonglonglonglong") == plg::string("longlonglonglonglonglonglonglonglonglonglonglong"));
	 }

	 SECTION("string operator+(string::const_pointer lhs, string&& rhs)") {
		  REQUIRE("small" + plg::string("small") == plg::string("smallsmall"));
		  REQUIRE("small" + plg::string("smallsmallsmallsmallsma") == plg::string("smallsmallsmallsmallsmallsma"));
		  REQUIRE("small" + plg::string("longlonglonglonglonglong") == plg::string("smalllonglonglonglonglonglong"));

		  REQUIRE("smallsmallsmallsmallsma" + plg::string("small") == plg::string("smallsmallsmallsmallsmasmall"));
		  REQUIRE("smallsmallsmallsmallsma" + plg::string("smallsmallsmallsmallsma") == plg::string("smallsmallsmallsmallsmasmallsmallsmallsmallsma"));
		  REQUIRE("smallsmallsmallsmallsma" + plg::string("longlonglonglonglonglong") == plg::string("smallsmallsmallsmallsmalonglonglonglonglonglong"));

		  REQUIRE("longlonglonglonglonglong" + plg::string("small") == plg::string("longlonglonglonglonglongsmall"));
		  REQUIRE("longlonglonglonglonglong" + plg::string("smallsmallsmallsmallsma") == plg::string("longlonglonglonglonglongsmallsmallsmallsmallsma"));
		  REQUIRE("longlonglonglonglonglong" + plg::string("longlonglonglonglonglong") == plg::string("longlonglonglonglonglonglonglonglonglonglonglong"));
	 }

	 SECTION("string operator+(string::value_type lhs, string&& rhs)") {
		  REQUIRE('a' + plg::string("small") == plg::string("asmall"));
		  REQUIRE('a' + plg::string("smallsmallsmallsmallsma") == plg::string("asmallsmallsmallsmallsma"));
		  REQUIRE('a' + plg::string("longlonglonglonglonglong") == plg::string("alonglonglonglonglonglong"));
	 }

}
