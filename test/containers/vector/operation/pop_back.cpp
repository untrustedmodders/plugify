#include <catch_amalgamated.hpp>

#include <app/counter.hpp>
#include <app/vector_tester.hpp>
#include <plugifyvector.hpp>

TEST_CASE("vector operation > pop_back", "[vector]") {
	SECTION("pop_back") {
		Counter counts;
		INFO(counts);

		auto x = size_t();
		for (size_t i = 0; i < 10; ++i) {
			auto a = plg::vector<Counter::Obj>();
			auto b = plg::vector<Counter::Obj>();
			for (size_t j = 0; j < i; ++j) {
				a.emplace_back(x, counts);
				b.emplace_back(x, counts);
				assert_eq(a, b);
				++x;
			}
			for (size_t j = 0; j < i; ++j) {
				assert_eq(a, b);
				a.pop_back();
				b.pop_back();
				assert_eq(a, b);
			}
			assert_eq(a, b);
			REQUIRE(a.size() == 0);
			REQUIRE(a.empty());
		}
	}

	SECTION("pop_back_trivial") {
		auto x = size_t();
		for (size_t i = 0; i < 10; ++i) {
			auto a = plg::vector<size_t>();
			auto b = plg::vector<size_t>();
			for (size_t j = 0; j < i; ++j) {
				a.emplace_back(x);
				b.emplace_back(x);
				assert_eq(a, b);
				++x;
			}
			for (size_t j = 0; j < i; ++j) {
				assert_eq(a, b);
				a.pop_back();
				b.pop_back();
				assert_eq(a, b);
			}
			assert_eq(a, b);
			REQUIRE(a.size() == 0);
			REQUIRE(a.empty());
		}
	}
}
