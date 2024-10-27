#include <catch2/catch_all.hpp>

#include <plugify/vector.h>
#include <app/counter.h>

#include <plugify/compat_format.h>

#include <string>

TEST_CASE("vector operation > emplace_back", "[vector]") {
	SECTION("emplace_back") {
		auto sv = plg::vector<std::string>();
		sv.emplace_back("hello");
		REQUIRE(sv.size() == 1);
		REQUIRE(sv.front() == "hello");

		sv.clear();
		REQUIRE(sv.empty());
		REQUIRE(sv.size() == 0);
	}

	SECTION("emplace_back_counts") {
		Counter counts;
		INFO(counts);
		{
			auto sv = plg::vector<Counter::Obj>();

			REQUIRE(sv.capacity() == 0);
			counts("begin");
			REQUIRE(counts.ctor == 0);

			for (size_t i = 0; i < 100; ++i) {
				sv.emplace_back(i, counts);
				counts("after emplace");
				REQUIRE(counts.ctor == i + 1);
				REQUIRE(counts.dtor == counts.moveCtor);
			}
		}
		counts("dtor");
		REQUIRE(counts.dtor == counts.ctor + counts.moveCtor);
	}
}
