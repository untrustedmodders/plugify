#include <catch_amalgamated.hpp>

#include <plugifystring.hpp>

TEST_CASE("string constructor", "[string]") {
	 SECTION("string() noexcept") {
		  plg::string a;
		  REQUIRE(a.empty());
		  REQUIRE(a.size() == 0);
		  REQUIRE(a.capacity() == 23);
	 }

	 SECTION("string(string const& other, size_type pos)") {
		  {
				plg::string a;
				plg::string b{a, 0};
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b == "");
		  }
		  {
				plg::string a{"toto"};
				plg::string b{a, 1};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b == "oto");
				REQUIRE(b.size() == 3);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a{"toto"};
				plg::string b{a, 3};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b == "o");
				REQUIRE(b.size() == 1);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a{"toto"};
				plg::string b{a, 4};
				REQUIRE(b.empty());
				REQUIRE(b == "");
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				plg::string b{a, 4};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b == "longlonglonglonglonglonglong");
				REQUIRE(b.size() == 28);
				REQUIRE(b.capacity() == 28);
		  }
	 }

	 SECTION("string(size_type count, value_type ch)") {
		  {
				plg::string a(0, 'a');
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE(a == "");
		  }
		  {
				plg::string a(1, 'a');
				REQUIRE_FALSE(a.empty());
				REQUIRE(a.size() == 1);
				REQUIRE(a.capacity() == 23);
				REQUIRE(a == "a");
		  }
		  {
				plg::string a(2, 'a');
				REQUIRE_FALSE(a.empty());
				REQUIRE(a.size() == 2);
				REQUIRE(a.capacity() == 23);
				REQUIRE(a == "aa");
		  }
		  {
				plg::string a(23, 'a');
				REQUIRE_FALSE(a.empty());
				REQUIRE(a.size() == 23);
				REQUIRE(a.capacity() == 23);
				REQUIRE(a == "aaaaaaaaaaaaaaaaaaaaaaa");
		  }
		  {
				plg::string a(24, 'a');
				REQUIRE_FALSE(a.empty());
				REQUIRE(a.size() == 24);
				REQUIRE(a.capacity() == 24);
				REQUIRE(a == "aaaaaaaaaaaaaaaaaaaaaaaa");
		  }
	 }

	 SECTION("string(string const& other, size_type pos, size_type count)") {
		  {
				plg::string a;
				plg::string b{a, 0, 0};
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b == "");
		  }
		  {
				plg::string a{"toto"};
				plg::string b{a, 1, 2};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b == "ot");
				REQUIRE(b.size() == 2);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a{"toto"};
				plg::string b{a, 3, 1};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b == "o");
				REQUIRE(b.size() == 1);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a{"toto"};
				plg::string b{a, 4, plg::string::npos};
				REQUIRE(b.empty());
				REQUIRE(b == "");
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong"};
				plg::string b{a, 4, 24};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b == "longlonglonglonglonglong");
				REQUIRE(b.size() == 24);
				REQUIRE(b.capacity() == 24);
		  }
	 }

	 SECTION("string(value_type const* s, size_type count) noexcept") {
		  plg::string a{"small", 5};
		  REQUIRE_FALSE(a.empty());
		  REQUIRE(a.size() == 5);
		  REQUIRE(a.capacity() == 23);
		  REQUIRE(a[0] == 's');
		  REQUIRE(a[4] == 'l');

		  plg::string b{"longlonglonglonglonglonglonglong", 32};
		  REQUIRE_FALSE(b.empty());
		  REQUIRE(b.size() == 32);
		  REQUIRE(b.capacity() == 32);
		  REQUIRE(b[0] == 'l');
		  REQUIRE(b[31] == 'g');

		  plg::string const c{"longlonglonglonglonglonglonglong", 0};
		  REQUIRE(c.empty());
		  REQUIRE(c.size() == 0);
		  REQUIRE(c.capacity() == 23);
	 }

	 SECTION("string(string const& other)") {
		  plg::string const a{"small", 5};
		  plg::string b{a};
		  REQUIRE_FALSE(b.empty());
		  REQUIRE(b.size() == 5);
		  REQUIRE(b.capacity() == 23);
		  REQUIRE(b[0] == 's');
		  REQUIRE(b[4] == 'l');
		  REQUIRE(a[0] == 's');
		  REQUIRE(a[4] == 'l');

		  plg::string c{"longlonglonglonglonglonglonglong", 32};
		  plg::string const d{c};
		  REQUIRE_FALSE(d.empty());
		  REQUIRE(d.size() == 32);
		  REQUIRE(d.capacity() == 32);
		  REQUIRE(d[0] == 'l');
		  REQUIRE(d[31] == 'g');
		  REQUIRE(c[0] == 'l');
		  REQUIRE(c[31] == 'g');
	 }

	 SECTION("string(string&& other) noexcept") {
		  plg::string a{"small", 5};
		  plg::string b{std::move(a)};
		  REQUIRE_FALSE(b.empty());
		  REQUIRE(b.size() == 5);
		  REQUIRE(b.capacity() == 23);
		  REQUIRE(b[0] == 's');
		  REQUIRE(b[4] == 'l');
		  REQUIRE(a.empty());
		  REQUIRE(a.size() == 0);
		  REQUIRE(a.capacity() == 23);

		  plg::string c{"longlonglonglonglonglonglonglong", 32};
		  plg::string const d{std::move(c)};
		  REQUIRE_FALSE(d.empty());
		  REQUIRE(d.size() == 32);
		  REQUIRE(d.capacity() == 32);
		  REQUIRE(d[0] == 'l');
		  REQUIRE(d[31] == 'g');
		  REQUIRE(c.empty());
		  REQUIRE(c.size() == 0);
		  REQUIRE(c.capacity() == 23);
	 }

	 SECTION("string(InputIt first, InputIt last)") {
		  {
				char const data[] {'t', 'o', 't', 'o'};
				plg::string a{&data[0], &data[0] + 4};
				REQUIRE_FALSE(a.empty());
				REQUIRE(a.size() == 4);
				REQUIRE(a.capacity() == 23);
				REQUIRE(a[0] == 't');
				REQUIRE(a[3] == 'o');
		  }
		  {
				char const data[] {'l', 'o', 'n', 'g', 'l', 'o', 'n', 'g', 'l', 'o', 'n', 'g', 'l', 'o', 'n', 'g', 'l', 'o', 'n', 'g', 'l', 'o', 'n', 'g', 'l', 'o', 'n', 'g', 'l' , 'o', 'n', 'g'};
				plg::string a{&data[0], &data[0] + 32};
				REQUIRE_FALSE(a.empty());
				REQUIRE(a.size() == 32);
				REQUIRE(a.capacity() == 32);
				REQUIRE(a[0] == 'l');
				REQUIRE(a[31] == 'g');
		  }
	 }

}
