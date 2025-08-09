#include <catch_amalgamated.hpp>

#include <plg/vector.hpp>

#include <string>

TEST_CASE("vector operation > push_back", "[vector]") {
	SECTION("push_back") {
		auto sv = plg::vector<uint32_t>();
		for (uint32_t i = 0; i < 1000; ++i) {
			REQUIRE(sv.size() == i);
			sv.push_back(i);
			REQUIRE(sv.size() == i + 1);
			REQUIRE(sv.front() == 0);
			REQUIRE(sv.back() == i);

			for (uint32_t j = 0; j < i; ++j) {
				REQUIRE(sv[j] == j);
			}
		}

		uint32_t i = 0;
		for (auto const& val : sv) {
			REQUIRE(val == i);
			++i;
		}
	}

	// tests if objects are properly moved
	SECTION("push_back_and_iterating") {
		auto sv = plg::vector<std::string>();
		for (auto i = 0; i < 50; ++i) {
			sv.push_back(std::to_string(i));
		}

		auto i = 0;
		for (auto const& str : sv) {
			REQUIRE(str == std::to_string(i));
			++i;
		}
	}
}
