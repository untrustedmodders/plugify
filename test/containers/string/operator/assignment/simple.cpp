#include <catch_amalgamated.hpp>

#include <plg/string.hpp>

TEST_CASE("string operator > assignment > simple", "[string]") {

	 SECTION("string& operator=(string const& other)") {
		  {
				plg::string a;
				plg::string b;
				b = a;
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a;
				plg::string b{"small", 5};
				b = a;
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a{"small", 5};
				plg::string b;
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }
		  {
				plg::string a{"small", 5};
				plg::string b{"small", 5};
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }

		  {
				plg::string a;
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = a;
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong", 32};
				plg::string b;
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 32);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[31] == 'g');
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong", 32};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 32);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[31] == 'g');
		  }

		  {
				plg::string a{"longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglong", 64};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 64);
				REQUIRE(b.capacity() == 64);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[63] == 'g');
		  }

		  {
				plg::string a{"longlonglonglonglonglong", 24};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 24);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[23] == 'g');
		  }

		  {
				plg::string a{"small", 5};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }
	 }

	 SECTION("string& operator=(string&& other) noexcept") {
		  {
				plg::string a;
				plg::string b;
				b = std::move(a);
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a;
				plg::string b{"small", 5};
				b = std::move(a);
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a{"small", 5};
				plg::string b;
				b = std::move(a);
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }
		  {
				plg::string a{"small", 5};
				plg::string b{"small", 5};
				b = std::move(a);
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }

		  {
				plg::string a;
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = std::move(a);
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong", 32};
				plg::string b;
				b = std::move(a);
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 32);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[31] == 'g');
		  }
		  {
				plg::string a{"longlonglonglonglonglonglonglong", 32};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = std::move(a);
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 32);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[31] == 'g');
		  }

		  {
				plg::string a{"longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglong", 64};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = std::move(a);
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 64);
				REQUIRE(b.capacity() == 64);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[63] == 'g');
		  }

		  {
				plg::string a{"longlonglonglonglonglong", 24};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = std::move(a);
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 24);
				REQUIRE(b.capacity() == 24);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[23] == 'g');
		  }

		  {
				plg::string a{"small", 5};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = std::move(a);
				REQUIRE(a.empty());
				REQUIRE(a.size() == 0);
				REQUIRE(a.capacity() == 23);
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }
	 }

	 SECTION("string& operator=(const_pointer s)") {
		  {
				plg::string b;
				b = "";
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string b{"small", 5};
				b = "";
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string b;
				b = "small";
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }
		  {
				plg::string b{"small", 5};
				b = "small";
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }

		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = "";
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string b;
				b = "longlonglonglonglonglonglonglong";
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 32);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[31] == 'g');
		  }
		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = "longlonglonglonglonglonglonglong";
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 32);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[31] == 'g');
		  }

		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglong";
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 64);
				REQUIRE(b.capacity() == 64);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[63] == 'g');
		  }

		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = "longlonglonglonglonglong";
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 24);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[23] == 'g');
		  }

		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = "small";
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }
	 }

	 SECTION("string& operator=(T const& t)") {
		  {
				std::string_view a;
				plg::string b;
				b = a;
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				std::string_view a;
				plg::string b{"small", 5};
				b = a;
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				std::string_view a{"small", 5};
				plg::string b;
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }
		  {
				std::string_view a{"small", 5};
				plg::string b{"small", 5};
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }

		  {
				std::string_view a;
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = a;
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				std::string_view a{"longlonglonglonglonglonglonglong", 32};
				plg::string b;
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 32);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[31] == 'g');
		  }
		  {
				std::string_view a{"longlonglonglonglonglonglonglong", 32};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 32);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[31] == 'g');
		  }

		  {
				std::string_view a{"longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglong", 64};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 64);
				REQUIRE(b.capacity() == 64);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[63] == 'g');
		  }

		  {
				std::string_view a{"longlonglonglonglonglong", 24};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 24);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[23] == 'g');
		  }

		  {
				std::string_view a{"small", 5};
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = a;
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }
	 }

	 SECTION("string& operator=(value_type ch)") {
		  {
				plg::string b;
				b = 'a';
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 1);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b == "a");
		  }
		  {
				plg::string b{"small", 5};
				b = 'a';
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 1);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b == "a");
		  }

		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = 'a';
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 1);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b == "a");
		  }
	 }

	 SECTION("string& operator=(std::initializer_list<value_type> ilist)") {
		  {
				plg::string b;
				b = std::initializer_list<char>{};
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string b{"small", 5};
				b = std::initializer_list<char>{};
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string b;
				b = std::initializer_list<char>{'s','m','a','l','l'};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }
		  {
				plg::string b{"small", 5};
				b = {'s','m','a','l','l'};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }

		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = {};
				REQUIRE(b.empty());
				REQUIRE(b.size() == 0);
				REQUIRE(b.capacity() == 23);
		  }
		  {
				plg::string b;
				b = {'l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g'};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 32);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[31] == 'g');
		  }
		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = {'l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g'};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 32);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[31] == 'g');
		  }

		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = {'l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g'};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 64);
				REQUIRE(b.capacity() == 64);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[63] == 'g');
		  }

		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = {'l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g','l','o','n','g'};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 24);
				REQUIRE(b.capacity() == 32);
				REQUIRE(b[0] == 'l');
				REQUIRE(b[23] == 'g');
		  }

		  {
				plg::string b{"longlonglonglonglonglonglonglong", 32};
				b = {'s','m','a','l','l'};
				REQUIRE_FALSE(b.empty());
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 23);
				REQUIRE(b[0] == 's');
				REQUIRE(b[4] == 'l');
		  }
	 }

}
