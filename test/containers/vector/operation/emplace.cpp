#include <catch_amalgamated.hpp>

#include <app/counter.hpp>
#include <app/vec_tester.hpp>
#include <plugify/vector.hpp>

#include <vector>

TEST_CASE("vector operation > emplace", "[vector]") {
	SECTION("emplace") {
		auto counts = Counter();
		INFO(counts);

		auto a = plg::vector<Counter::Obj>();

		a.emplace(a.cend(), 123, counts);
		REQUIRE(a.size() == 1);
		REQUIRE(a[0].get() == 123);
	}

	SECTION("emplace_checked") {
		auto counts = Counter();

		for (size_t s = 0; s < 6; ++s) {
			auto vc = VecTester<Counter::Obj>();
			for (size_t ins = 0; ins < s; ++ins) {
				vc.emplace_back(ins, counts);
			}

			for (size_t i = 0; i <= vc.size(); ++i) {
				auto x = vc;
				x.emplace_at(i, 999, counts);
			}
		}
	}
}

