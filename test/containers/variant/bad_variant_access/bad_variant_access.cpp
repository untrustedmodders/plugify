#include <catch_amalgamated.hpp>

#include <plugify/variant.hpp>

TEST_CASE("variant > bad_variant_access", "[variant]") {
	SECTION("class bad_variant_access : public exception") {
		static_assert(std::is_base_of<std::exception, plg::bad_variant_access>::value);
		static_assert(noexcept(plg::bad_variant_access{"hello"}), "must be noexcept");
		static_assert(noexcept(plg::bad_variant_access{"hello"}.what()), "must be noexcept");
		plg::bad_variant_access ex;
		REQUIRE(ex.what());
	}
}