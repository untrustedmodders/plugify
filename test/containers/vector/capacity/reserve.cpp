#include <catch2/catch_all.hpp>

#include <plugify/vector.h>
#include <app/counter.h>

#include <stdexcept>

TEST_CASE("vector capacity > reserve", "[vector]") {
	SECTION("reserve") {
		Counter counts;
		auto a = plg::vector<Counter::Obj>();
		REQUIRE(a.capacity() < 100);
		a.reserve(100);
		REQUIRE(a.capacity() == 100);
		a.reserve(192);
		REQUIRE(a.capacity() == 200);
		a.reserve(193);
		REQUIRE(a.capacity() == 200);

		// going down does nothing
		a.reserve(0);
		REQUIRE(a.capacity() == 200);
		a.push_back(Counter::Obj(123, counts));
		REQUIRE(a.size() == 1);
		a.reserve(1);
		REQUIRE(a.size() == 1);
		REQUIRE(a.capacity() == 200);

		// reserve even more, with data
		a.reserve(385);
		REQUIRE(a.capacity() == 400);
		REQUIRE(a.size() == 1);
		REQUIRE(a[0].get() == 123);
	}

	SECTION("reserve_direct") {
		auto counts = Counter();
		auto a = plg::vector<Counter::Obj>();
		a.emplace_back(1234, counts);
		for (size_t i = 0; i < 100; ++i) {
			a.reserve(i);
		}
	}
}