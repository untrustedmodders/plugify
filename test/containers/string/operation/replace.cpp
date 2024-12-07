#include <catch_amalgamated.hpp>

#include <plugify/string.hpp>

TEST_CASE("string operation > replace", "[string]") {

	 SECTION("string& replace(size_type pos, size_type count, string const& str)") {
		  {
				plg::string a;
				plg::string const b;
				a.replace(0, 2, b);
				REQUIRE(a == "");
		  }
		  {
				plg::string a;
				plg::string const b{"aaaa"};
				a.replace(0, 2, b);
				REQUIRE(a == "aaaa");
		  }
		  {
				plg::string a{"abcdef"};
				plg::string const b{"wxyz"};
				a.replace(0, 2, b);
				REQUIRE(a == "wxyzcdef");
		  }
		  {
				plg::string a{"abcdef"};
				plg::string const b{"longlonglonglonglonglonglonglong"};
				a.replace(2, 2, b);
				REQUIRE(a == "ablonglonglonglonglonglonglonglongef");
		  }
		  {
				plg::string a{"abcdef"};
				plg::string const b{"longlonglonglonglonglonglonglong"};
				a.replace(2, 0, b);
				REQUIRE(a == "ablonglonglonglonglonglonglonglongcdef");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				plg::string const b{"abcd"};
				a.replace(4, 4, b);
				REQUIRE(a == "longabcdlonglonglonglonglonglong");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				plg::string const b{"totototototototototototototototo"};
				a.replace(4, 32, b);
				REQUIRE(a == "longtotototototototototototototototo");
		  }
	 }

	 SECTION("string& replace(const_iterator first, const_iterator last, string const& str)") {
		  {
				plg::string a;
				plg::string const b;
				a.replace(a.cbegin(), a.cend(), b);
				REQUIRE(a == "");
		  }
		  {
				plg::string a;
				plg::string b{"aaaa"};
				a.replace(a.cbegin(), a.cend(), b);
				REQUIRE(a == "aaaa");
		  }
		  {
				plg::string a{"abcdef"};
				plg::string b{"wxyz"};
				a.replace(a.cbegin() + 2, a.cbegin() + 4, b);
				REQUIRE(a == "abwxyzef");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				plg::string const b{"abcd"};
				a.replace(a.begin() + 4, a.begin() + 8, b);
				REQUIRE(a == "longabcdlonglonglonglonglonglong");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				plg::string const b{"totototototototototototototototo"};
				a.replace(a.cbegin(), a.cend(), b);
				REQUIRE(a == "totototototototototototototototo");
		  }
	 }

	 SECTION("string& replace(size_type pos, size_type count, string const& str, size_type pos2, size_type count2 = npos)") {
	 }

	 SECTION("string& replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2)") {
	 }

	 SECTION("string& replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2)") {
	 }

	 SECTION("string& replace(size_type pos, size_type count, const_pointer cstr, size_type count2)") {
	 }

	 SECTION("string& replace(const_iterator first, const_iterator last, const_pointer cstr, size_type count2)") {
	 }

	 SECTION("string& replace(size_type pos, size_type count, const_pointer cstr)") {
	 }

	 SECTION("string& replace(const_iterator first, const_iterator last, const_pointer cstr)") {
	 }

	 SECTION("string& replace(size_type pos, size_type count, size_type count2, value_type ch)") {
	 }

	 SECTION("string& replace(const_iterator first, const_iterator last, size_type count2, value_type ch)") {
	 }

	 SECTION("string& replace(const_iterator first, const_iterator last, std::initializer_list<value_type> ilist)") {
	 }

	 SECTION("string& replace(size_type pos, size_type count, T const& t)") {
	 }

	 SECTION("string& replace(const_iterator first, const_iterator last, T const& t)") {
	 }

	 SECTION("string& replace(size_type pos, size_type count, T const& t, size_type pos2, size_type count2 = npos)") {
	 }

}
