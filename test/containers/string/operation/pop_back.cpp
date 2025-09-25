#include <catch_amalgamated.hpp>

#include <plg/string.hpp>

TEST_CASE("string operation > pop_back", "[string]") {

	 SECTION("void pop_back()") {
		  {
				plg::string a{"toto"};
				a.pop_back();
				REQUIRE(a == "tot");
				REQUIRE(a.size() == 3);
				REQUIRE(a.capacity() == 23);
		  }
		  {
				plg::string a{"totototototototototototo"};
				a.pop_back();
				REQUIRE(a == "tototototototototototot");
				REQUIRE(a.size() == 23);
				REQUIRE(a.capacity() == 31);
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				a.pop_back();
				REQUIRE(a == "longlonglonglonglonglonglonglon");
				REQUIRE(a.size() == 31);
				REQUIRE(a.capacity() == 39);
		  }
	 }

}
