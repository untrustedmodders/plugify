#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

#include <memory_resource>

TEST_CASE("vector access > get_allocator", "[vector]") {

	SECTION("get_allocator") {
		using alloc_type = std::pmr::polymorphic_allocator<int>;
		plg::pmr::vector<int> v{};

		REQUIRE(v.empty());
		REQUIRE(alloc_type{ } == v.get_allocator());
		REQUIRE(alloc_type{ } == v.get_allocator());

		alloc_type alloc;
		plg::vector<short, alloc_type> w(alloc);

		REQUIRE(w.empty());
		REQUIRE(alloc == w.get_allocator());
	}
}
