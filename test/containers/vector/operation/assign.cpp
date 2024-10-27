#include <catch2/catch_all.hpp>

#include <plugify/vector.h>

#include <ostream>
#include <string>
#include <string_view>

using namespace std::literals;

TEST_CASE("vector operation > assign", "[vector]") {
	SECTION("assign_size_value") {
		auto sv = plg::vector<char>();
		sv.push_back('x');

		sv.assign(1000, 'a');
		REQUIRE(sv.size() == 1000);
		for (auto c : sv) {
			REQUIRE(c == 'a');
		}
	}

	SECTION("assign_iterators") {
		auto str = "hello world!"sv;

		auto sv = plg::vector<char>(100, 'x');
		sv.assign(str.begin(), str.end());

		REQUIRE(sv.size() == str.size());
		REQUIRE(str == std::string_view(sv.data(), sv.size()));
	}

	SECTION("assign_initializer_list") {
		auto sv = plg::vector<char>(100, 'x');
		sv.assign({'a', 'b', 'c'});
		REQUIRE(sv.size() == 3);
		REQUIRE(sv[0] == 'a');
		REQUIRE(sv[1] == 'b');
		REQUIRE(sv[2] == 'c');
	}
}
