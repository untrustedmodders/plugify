#include <catch_amalgamated.hpp>

#include <plg/vector.hpp>

#include <app/counter.hpp>

#include <numeric>
#include <ostream>
#include <stdexcept>
#include <string_view>

TEST_CASE("algorithm", "[vector]") {
	SECTION("std_erase") {
		auto sv = plg::vector<char>(10);
		std::iota(sv.begin(), sv.end(), '0');
		REQUIRE("0123456789" == std::string_view(sv.data(), sv.size()));

		REQUIRE(1 == plg::erase(sv, '3'));
		REQUIRE("012456789" == std::string_view(sv.data(), sv.size()));

		REQUIRE(0 == plg::erase(sv, '3'));
		REQUIRE("012456789" == std::string_view(sv.data(), sv.size()));

		sv.insert(sv.begin(), 'a');
		sv.insert(sv.begin() + sv.size() / 2, 'a');
		sv.insert(sv.end(), 'a');
		REQUIRE("a0124a56789a" == std::string_view(sv.data(), sv.size()));

		REQUIRE(3 == plg::erase(sv, 'a'));
		REQUIRE("012456789" == std::string_view(sv.data(), sv.size()));

		auto num_erased = plg::erase_if(sv, [](char x) {
			return (x - '0') % 2 == 0;
		});
		REQUIRE(num_erased == 5);
		REQUIRE("1579" == std::string_view(sv.data(), sv.size()));

		sv.erase(sv.begin() + 1, sv.end() - 1);
		REQUIRE("19" == std::string_view(sv.data(), sv.size()));
	}
}
