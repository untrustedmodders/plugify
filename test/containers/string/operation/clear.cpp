#include <catch_amalgamated.hpp>

#include <plg/string.hpp>


TEST_CASE("string operation > clear", "[string]") {

	 SECTION("void clear() noexcept") {
		  plg::string a{"toto", 4};
		  a.clear();
		  REQUIRE(a.empty());
		  REQUIRE(a.size() == 0);
		  REQUIRE(a.capacity() == 23);

		  plg::string b{"longlonglonglonglonglonglonglong", 32};
		  b.clear();
		  REQUIRE(b.empty());
		  REQUIRE(b.size() == 0);
		  REQUIRE(b.capacity() == 32);
	 }

}

