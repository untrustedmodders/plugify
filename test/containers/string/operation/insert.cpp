#include <catch_amalgamated.hpp>

#include <plugifystring.hpp>

TEST_CASE("string operation > insert", "[string]") {

	 SECTION("string& insert(size_type index, size_type count, value_type ch)") {
		  {
				plg::string a;
				a.insert(static_cast<plg::string::size_type>(0), static_cast<plg::string::size_type>(1), 'a');
				REQUIRE(a == "a");
		  }
		  {
				plg::string a{"small"};
				a.insert(static_cast<plg::string::size_type>(0), static_cast<plg::string::size_type>(1), 'a');
				REQUIRE(a == "asmall");
		  }
		  {
				plg::string a{"small"};
				a.insert(static_cast<plg::string::size_type>(2), static_cast<plg::string::size_type>(4), 'b');
				REQUIRE(a == "smbbbball");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				a.insert(static_cast<plg::string::size_type>(23), static_cast<plg::string::size_type>(4), 'b');
				REQUIRE(a == "smallsmallsmallsmallsmabbbb");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				a.insert(static_cast<plg::string::size_type>(16), static_cast<plg::string::size_type>(4), 'b');
				REQUIRE(a == "longlonglonglongbbbblonglonglonglong");
		  }
	 }

	 SECTION("string& insert(size_type index, const_pointer s)") {
		  {
				plg::string a;
				a.insert(static_cast<plg::string::size_type>(0), "aaaa");
				REQUIRE(a == "aaaa");
		  }
		  {
				plg::string a{"small"};
				a.insert(static_cast<plg::string::size_type>(0), "a");
				REQUIRE(a == "asmall");
		  }
		  {
				plg::string a{"small"};
				a.insert(static_cast<plg::string::size_type>(0), a.c_str());
				REQUIRE(a == "smallsmall");
		  }
		  {
				plg::string a{"small"};
				a.insert(static_cast<plg::string::size_type>(2), "bbbb");
				REQUIRE(a == "smbbbball");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				a.insert(static_cast<plg::string::size_type>(23), "bbbb");
				REQUIRE(a == "smallsmallsmallsmallsmabbbb");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				a.insert(static_cast<plg::string::size_type>(16), "bbbb");
				REQUIRE(a == "longlonglonglongbbbblonglonglonglong");
		  }
	 }

	 SECTION("string& insert(size_type index, const_pointer s, size_type count)") {
		  {
				plg::string a;
				a.insert(static_cast<plg::string::size_type>(0), "aaaa", static_cast<plg::string::size_type>(2));
				REQUIRE(a == "aa");
		  }
		  {
				plg::string a{"small"};
				a.insert(static_cast<plg::string::size_type>(0), "a", static_cast<plg::string::size_type>(1));
				REQUIRE(a == "asmall");
		  }
		  {
				plg::string a{"small"};
				a.insert(static_cast<plg::string::size_type>(2), "bbbb", static_cast<plg::string::size_type>(2));
				REQUIRE(a == "smbball");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				a.insert(static_cast<plg::string::size_type>(23), "bbbb", static_cast<plg::string::size_type>(2));
				REQUIRE(a == "smallsmallsmallsmallsmabb");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				a.insert(static_cast<plg::string::size_type>(16), "bbbb", static_cast<plg::string::size_type>(2));
				REQUIRE(a == "longlonglonglongbblonglonglonglong");
		  }
	 }

	 SECTION("string& insert(size_type index, const string& str)") {
		  {
				plg::string a;
				plg::string const b{"aa"};
				a.insert(static_cast<plg::string::size_type>(0), b);
				REQUIRE(a == "aa");
		  }
		  {
				plg::string a{"small"};
				plg::string const b{"a"};
				a.insert(static_cast<plg::string::size_type>(0), b);
				REQUIRE(a == "asmall");
		  }
		  {
				plg::string a{"small"};
				plg::string b{"bb"};
				a.insert(static_cast<plg::string::size_type>(2), b);
				REQUIRE(a == "smbball");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				plg::string b{"bb"};
				a.insert(static_cast<plg::string::size_type>(23), b);
				REQUIRE(a == "smallsmallsmallsmallsmabb");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				plg::string b{"bb"};
				a.insert(static_cast<plg::string::size_type>(16), b);
				REQUIRE(a == "longlonglonglongbblonglonglonglong");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				plg::string b{"totototototototototototototototo"};
				a.insert(static_cast<plg::string::size_type>(16), b);
				REQUIRE(a == "longlonglonglongtotototototototototototototototolonglonglonglong");
		  }
		  {
				plg::string a{"small"};
				a.insert(static_cast<plg::string::size_type>(2), a);
				REQUIRE(a == "smsmallall");
		  }
	 }

	 SECTION("string& insert(size_type index, const string& str, size_type index_str, size_type count = npos)") {
		  {
				plg::string a;
				plg::string const b{"abcd"};
				a.insert(static_cast<plg::string::size_type>(0), b, static_cast<plg::string::size_type>(2), static_cast<plg::string::size_type>(1));
				REQUIRE(a == "c");
		  }
		  {
				plg::string a{"small"};
				plg::string const b{"abcd"};
				a.insert(static_cast<plg::string::size_type>(0), b, static_cast<plg::string::size_type>(2), static_cast<plg::string::size_type>(10));
				REQUIRE(a == "cdsmall");
		  }
		  {
				plg::string a{"small"};
				plg::string b{"abcd"};
				a.insert(static_cast<plg::string::size_type>(2), b, static_cast<plg::string::size_type>(1), static_cast<plg::string::size_type>(2));
				REQUIRE(a == "smbcall");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				plg::string b{"abcd"};
				a.insert(static_cast<plg::string::size_type>(23), b, static_cast<plg::string::size_type>(4));
				REQUIRE(a == "smallsmallsmallsmallsma");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				plg::string b{"abcd"};
				a.insert(static_cast<plg::string::size_type>(16), b, static_cast<plg::string::size_type>(1));
				REQUIRE(a == "longlonglonglongbcdlonglonglonglong");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				plg::string b{"totototototototototototototototo"};
				a.insert(static_cast<plg::string::size_type>(16), b, static_cast<plg::string::size_type>(1), static_cast<plg::string::size_type>(4));
				REQUIRE(a == "longlonglonglongototlonglonglonglong");
		  }
		  {
				plg::string a{"small"};
				a.insert(static_cast<plg::string::size_type>(2), a, static_cast<plg::string::size_type>(1), static_cast<plg::string::size_type>(4));
				REQUIRE(a == "smmallall");
		  }
	 }

	 SECTION("iterator insert(const_iterator pos, value_type ch)") {
		  {
				plg::string a;
				a.insert(a.data(), 'a');
				REQUIRE(a == "a");
		  }
		  {
				plg::string a{"small"};
				a.insert(a.data() + 2, 'b');
				REQUIRE(a == "smball");
		  }
		  {
				plg::string a{"small"};
				a.insert(a.data() + 1, 'b');
				REQUIRE(a == "sbmall");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				a.insert(a.data() + 5, 'b');
				REQUIRE(a == "smallbsmallsmallsmallsma");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				a.insert(a.data() + 4, 'b');
				REQUIRE(a == "longblonglonglonglonglonglonglong");
		  }
	 }

	 SECTION("iterator insert(const_iterator pos, size_type count, value_type ch)") {
		  {
				plg::string a;
				a.insert(a.data(), static_cast<plg::string::size_type>(32), 'a');
				REQUIRE(a == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
		  }
		  {
				plg::string a{"small"};
				a.insert(a.data() + 2, static_cast<plg::string::size_type>(4), 'b');
				REQUIRE(a == "smbbbball");
		  }
		  {
				plg::string a{"small"};
				a.insert(a.data() + 1, static_cast<plg::string::size_type>(1), 'b');
				REQUIRE(a == "sbmall");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				a.insert(a.data() + 5, 'b');
				REQUIRE(a == "smallbsmallsmallsmallsma");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				a.insert(a.data() + 4, static_cast<plg::string::size_type>(3), 'b');
				REQUIRE(a == "longbbblonglonglonglonglonglonglong");
		  }
	 }

	 SECTION("iterator insert(const_iterator pos, InputIt first, InputIt last)") {
		  {
				plg::string a;
				auto const data {"aaaa"};
				a.insert(a.data(), &data[0], &data[0] + 4);
				REQUIRE(a == "aaaa");
		  }
		  {
				plg::string a {"small"};
				auto const data {"bbbb"};
				a.insert(a.data() + 2, &data[0], &data[0] + 4);
				REQUIRE(a == "smbbbball");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				auto const data {"longlonglonglonglonglonglonglong"};
				a.insert(a.data() + 5, &data[0], &data[0] + 32);
				REQUIRE(a == "smalllonglonglonglonglonglonglonglongsmallsmallsmallsma");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				auto const data {"totototototototototototototototo"};
				a.insert(a.data() + 4, &data[0], &data[0] + 32);
				REQUIRE(a == "longtotototototototototototototototolonglonglonglonglonglonglong");
		  }
	 }

	 SECTION("iterator insert(const_iterator pos, std::initializer_list<value_type> ilist)") {
		  {
				plg::string a;
				a.insert(a.data(), {'a', 'a', 'a', 'a'});
				REQUIRE(a == "aaaa");
		  }
		  {
				plg::string a {"small"};
				a.insert(a.data() + 2, {'b', 'b', 'b', 'b'});
				REQUIRE(a == "smbbbball");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				a.insert(a.data() + 5, {'l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g'});
				REQUIRE(a == "smalllonglonglonglonglonglonglonglongsmallsmallsmallsma");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				a.insert(a.data() + 4, {'t','o','t','o','t','o','t','o','t','o','t','o','t','o','t','o','t','o','t','o','t','o','t','o','t','o','t','o','t','o','t','o'});
				REQUIRE(a == "longtotototototototototototototototolonglonglonglonglonglonglong");
		  }
		  {
				plg::string a;
				a.insert(a.data(), std::initializer_list<char>{});
				REQUIRE(a == "");
		  }
	 }

	 SECTION("string& insert(size_type pos, T const& t)") {
		  {
				plg::string a;
				std::string_view const b{"aa"};
				a.insert(static_cast<plg::string::size_type>(0), b);
				REQUIRE(a == "aa");
		  }
		  {
				plg::string a{"small"};
				std::string_view const b{"a"};
				a.insert(static_cast<plg::string::size_type>(0), b);
				REQUIRE(a == "asmall");
		  }
		  {
				plg::string a{"small"};
				std::string_view b{"bb"};
				a.insert(static_cast<plg::string::size_type>(2), b);
				REQUIRE(a == "smbball");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				std::string_view b{"bb"};
				a.insert(static_cast<plg::string::size_type>(23), b);
				REQUIRE(a == "smallsmallsmallsmallsmabb");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				std::string_view b{"bb"};
				a.insert(static_cast<plg::string::size_type>(16), b);
				REQUIRE(a == "longlonglonglongbblonglonglonglong");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				std::string_view b{"totototototototototototototototo"};
				a.insert(static_cast<plg::string::size_type>(16), b);
				REQUIRE(a == "longlonglonglongtotototototototototototototototolonglonglonglong");
		  }
	 }

	 SECTION("string& insert(size_type index, T const& t, size_type index_str, size_type count = npos)") {
		  {
				plg::string a;
				std::string_view const b{"abcd"};
				a.insert(static_cast<plg::string::size_type>(0), b, static_cast<plg::string::size_type>(2), static_cast<plg::string::size_type>(1));
				REQUIRE(a == "c");
		  }
		  {
				plg::string a{"small"};
				std::string_view const b{"abcd"};
				a.insert(static_cast<plg::string::size_type>(0), b, static_cast<plg::string::size_type>(2), static_cast<plg::string::size_type>(10));
				REQUIRE(a == "cdsmall");
		  }
		  {
				plg::string a{"small"};
				std::string_view b{"abcd"};
				a.insert(static_cast<plg::string::size_type>(2), b, static_cast<plg::string::size_type>(1), static_cast<plg::string::size_type>(2));
				REQUIRE(a == "smbcall");
		  }
		  {
				plg::string a{"smallsmallsmallsmallsma"};
				std::string_view b{"abcd"};
				a.insert(static_cast<plg::string::size_type>(23), b, static_cast<plg::string::size_type>(4));
				REQUIRE(a == "smallsmallsmallsmallsma");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				std::string_view b{"abcd"};
				a.insert(static_cast<plg::string::size_type>(16), b, static_cast<plg::string::size_type>(1));
				REQUIRE(a == "longlonglonglongbcdlonglonglonglong");
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				std::string_view b{"totototototototototototototototo"};
				a.insert(static_cast<plg::string::size_type>(16), b, static_cast<plg::string::size_type>(1), static_cast<plg::string::size_type>(4));
				REQUIRE(a == "longlonglonglongototlonglonglonglong");
		  }
	 }

}
