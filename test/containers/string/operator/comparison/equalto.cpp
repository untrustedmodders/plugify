#include <catch_amalgamated.hpp>

#include <plugify/string.hpp>

TEST_CASE("string operator > comparison > equalto", "[string]") {

	 SECTION("bool operator==(string const& lhs, string const& rhs) noexcept") {
		  {
				plg::string a{"toto", 4};
				plg::string b{"toto", 4};
				REQUIRE(a == b);
		  }
		  {
				plg::string const a{"toto", 4};
				plg::string b{"tata", 4};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string a{"toto", 4};
				plg::string const b{"t", 1};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string a{"totototototototototototototototo", 32};
				plg::string b{"totototototototototototototototo", 32};
				REQUIRE(a == b);
		  }
		  {
				plg::string const a{"totototototototototototototototo", 32};
				plg::string b{"tatatatatatatatatatatatatatatata", 32};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string a{"totototototototototototototototo", 32};
				plg::string const b{"totototototototototototo", 24};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string a{"toto", 4};
				plg::string b{"totototototototototototototototo", 32};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string const a{"totototototototototototototototo", 32};
				plg::string b{"tata", 4};
				REQUIRE_FALSE(a == b);
		  }
	 }

	 SECTION("bool operator==(string const& lhs, string::const_pointer rhs) noexcept") {
		  {
				plg::string a{"toto", 4};
				plg::string::const_pointer b{"toto"};
				REQUIRE(a == b);
		  }
		  {
				plg::string const a{"toto", 4};
				plg::string::const_pointer b{"tata"};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string a{"toto", 4};
				plg::string::const_pointer const b{"t"};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string a{"totototototototototototototototo", 32};
				plg::string::const_pointer b{"totototototototototototototototo"};
				REQUIRE(a == b);
		  }
		  {
				plg::string const a{"totototototototototototototototo", 32};
				plg::string::const_pointer b{"tatatatatatatatatatatatatatatata"};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string a{"totototototototototototototototo", 32};
				plg::string::const_pointer const b{"totototototototototototo"};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string a{"toto", 4};
				plg::string::const_pointer b{"totototototototototototototototo"};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string const a{"totototototototototototototototo", 32};
				plg::string::const_pointer b{"tata"};
				REQUIRE_FALSE(a == b);
		  }
	 }

	 SECTION("bool operator==(string::const_pointer rhs, string const& lhs) noexcept") {
		  {
				plg::string::const_pointer a{"toto"};
				plg::string b{"toto", 4};
				REQUIRE(a == b);
		  }
		  {
				plg::string::const_pointer const a{"toto"};
				plg::string b{"tata", 4};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string::const_pointer a{"toto"};
				plg::string const b{"t", 1};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string::const_pointer a{"totototototototototototototototo"};
				plg::string b{"totototototototototototototototo", 32};
				REQUIRE(a == b);
		  }
		  {
				plg::string::const_pointer const a{"totototototototototototototototo"};
				plg::string b{"tatatatatatatatatatatatatatatata", 32};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string::const_pointer a{"totototototototototototototototo"};
				plg::string const b{"totototototototototototo", 24};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string::const_pointer a{"toto"};
				plg::string b{"totototototototototototototototo", 32};
				REQUIRE_FALSE(a == b);
		  }
		  {
				plg::string::const_pointer const a{"totototototototototototototototo"};
				plg::string b{"tata", 4};
				REQUIRE_FALSE(a == b);
		  }
	 }

}
