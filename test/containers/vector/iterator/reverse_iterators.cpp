#include <catch2/catch_all.hpp>

#include <plugify/vector.h>

#include <ostream>
#include <string_view>

using namespace std::literals;

TEST_CASE("vector iterator > begin", "[vector]") {
	SECTION("rbegin_rend") {
		auto a = plg::vector<char>();
		for (auto c: "hello world!"sv) {
			a.push_back(c);
		}
		auto it = a.rbegin();
		auto itc = a.crbegin();
		REQUIRE(*it == '!');
		REQUIRE(*itc == '!');
		++it;
		REQUIRE(*it == 'd');
		REQUIRE(*itc == '!');
		++itc;
		REQUIRE(*itc == 'd');
		*it = 'x';
		REQUIRE(*it == 'x');
		REQUIRE(*itc == 'x');

		auto const& ca = a;
		auto itc2 = ca.rbegin();
		auto v2 = plg::vector<char>();
		while (itc2 != ca.rend()) {
			v2.push_back(*itc2);
			++itc2;
		}
		REQUIRE(std::string_view(v2.data(), v2.size()) == "!xlrow olleh");
	}
}
