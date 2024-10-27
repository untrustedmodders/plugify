#include <catch2/catch_all.hpp>

#include <plugify/vector.h>
#include <plugify/compat_format.h>

#include <iostream>
#include <vector>

template <typename T>
void show(char const* what) {
    std::cout << std::format("{} sizeof({})\n", sizeof(T), what)
			  << std::format("{} capacity({})\n", T{}.capacity(), what)
			  << std::format("{} max_size({})\n\n", T{}.max_size(), what)
			  << std::endl;
}

TEST_CASE("comparisons", "[vector]") {
	SECTION("show_sizeof") {
		show<plg::vector<uint8_t>>("plg::vector<uint8_t>");
		show<plg::vector<uint8_t>>("plg::vector<uint8_t>");
	}
}
