#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

#include <app/counter.hpp>

#include <plugify/compat_format.hpp>

#include <forward_list>

namespace {
	struct Foo {
		Foo(int /*a*/, char /*b*/) {}
	};
} // namespace

TEST_CASE("vector constructor", "[vector]") {
	SECTION("ctor_default") {
		auto counts = Counter();
		auto sv = plg::vector<Counter::Obj>();
		REQUIRE(Counter::staticDefaultCtor == 0);
		REQUIRE(Counter::staticDtor == 0);

		// default ctor shouldn't be needed
		auto sv2 = plg::vector<Foo>();
		REQUIRE(sv2.size() == 0);
	}

	SECTION("ctor_count") {
		auto counts = Counter();
		INFO(counts);
		counts("begin");
		auto o = Counter::Obj(123, counts);
		counts("one o");

		// creates a vector with copies, no allocation yet
		REQUIRE(counts.ctor == 1);
		auto sv = plg::vector<Counter::Obj>(7, o);
		counts("ctor with 7");
		REQUIRE(counts.copyCtor == 7);
		REQUIRE(counts.moveCtor == 0);
		REQUIRE(sv.size() == 7);
		REQUIRE(sv.capacity() == 7);

		auto x = plg::vector<Counter::Obj>(3);
		REQUIRE(x.size() == 3);
		REQUIRE(x.capacity() == 3);
		REQUIRE(x[0] == Counter::Obj{});
	}

	SECTION("ctor_count_big") {
		auto sv = plg::vector<char>(100000, 'x');
		REQUIRE(sv.size() == 100000);
		for (auto c: sv) {
			REQUIRE(c == 'x');
		}
	}

	SECTION("ctor_default") {
		auto counts = Counter();
		INFO(counts);
		REQUIRE(counts.staticDefaultCtor == 0);
		counts("begin");
		//  no copies are made, just default ctor
		auto sv = plg::vector<Counter::Obj>(100);
		// auto sv = plg::vector<Counter::Obj>(100);
		counts("after 100");
		REQUIRE(Counter::staticDefaultCtor == 100);
	}

	SECTION("ctor_iterators") {
		auto str = std::string_view("hello world!");
		auto sv = plg::vector<char>(str.begin(), str.end());
		REQUIRE(sv.size() == str.size());
		for (size_t i = 0; i < sv.size(); ++i) {
			REQUIRE(sv[i] == str[i]);
		}
	}

	SECTION("ctor_not_random_access_iterator") {
		auto l = std::forward_list<char>();
		auto str = std::string_view("hello world!");
		for (auto it = str.rbegin(); it != str.rend(); ++it) {
			l.push_front(*it);
		}
		auto sv = plg::vector<char>(l.begin(), l.end());
		REQUIRE(sv.size() == str.size());
		for (size_t i = 0; i < sv.size(); ++i) {
			REQUIRE(sv[i] == str[i]);
		}
	}

	SECTION("ctor_copy") {
		auto counts = Counter();
		INFO(counts);

		auto sv = plg::vector<Counter::Obj>();
		for (char c = 'a'; c <= 'z'; ++c) {
			sv.emplace_back(c, counts);
		}
		REQUIRE(sv.size() == 26);

		auto svCpy(sv);
		REQUIRE(sv.size() == svCpy.size());
		for (size_t i = 0; i < sv.size(); ++i) {
			REQUIRE(sv[i] == svCpy[i]);
			REQUIRE(&sv[i] != &svCpy[i]);
		}

		sv.clear();
		REQUIRE(svCpy.size() == 26);
		auto svCpy2(sv);
		REQUIRE(svCpy2.size() == 0);
		REQUIRE(sv.size() == 0);
		REQUIRE(svCpy2.capacity() == 32);
	}

	SECTION("ctor_move") {
		auto counts = Counter();
		INFO(counts);
		counts("begin");
		auto sv = plg::vector<Counter::Obj>();
		for (size_t i = 0; i < 100; ++i) {
			sv.emplace_back(i, counts);
		}
		auto total_before = counts.total();
		counts("before move");
		auto sv2(std::move(sv));
		auto total_after = counts.total();
		counts("after move");
		REQUIRE(total_before == total_after);

		// I guarantee that a moved-from value is in the default constructed state. It just makes everything easier.
		REQUIRE(sv.empty());// NOLINT(hicpp-invalid-access-moved)
		REQUIRE(sv2.size() == 100);
	}

	SECTION("ctor_move_direct") {
		auto counts = Counter();
		INFO(counts);
		counts("begin");
		auto sv = plg::vector<Counter::Obj>();
		sv.emplace_back(1, counts);
		sv.emplace_back(2, counts);
		sv.emplace_back(3, counts);
		counts("3 emplace_back");
		REQUIRE(counts.ctor == 3);
		REQUIRE(counts.dtor == 3);
		REQUIRE(counts.moveCtor == 3);

		auto svMv = plg::vector<Counter::Obj>(std::move(sv));
		counts("after moved");
		REQUIRE(svMv.size() == 3);
		REQUIRE(svMv.capacity() == 4);
		REQUIRE(counts.ctor == 3);
		REQUIRE(counts.moveCtor == 3);
		REQUIRE(counts.dtor == 3);

		REQUIRE(svMv[0].get() == 1);
		REQUIRE(svMv[1].get() == 2);
		REQUIRE(svMv[2].get() == 3);
		REQUIRE(sv.empty());// NOLINT(hicpp-invalid-access-moved)
	}

	SECTION("ctor_move_empty_capacity") {
		auto sv = plg::vector<char>(1000, 'x');
		auto c = sv.capacity();
		sv.clear();
		REQUIRE(sv.capacity() == c);
		REQUIRE(sv.empty());

		auto sv2 = plg::vector<char>(std::move(sv));
		REQUIRE(sv.capacity() == 0);// NOLINT(hicpp-invalid-access-moved)
		REQUIRE(sv.empty());
		REQUIRE(sv2.capacity() == c);
		REQUIRE(sv2.size() == 0);
	}

	SECTION("ctor_initializer_list") {
		auto sv = plg::vector<char>{{'h', 'e', 'l', 'l', 'o'}};
		REQUIRE(std::string(sv.begin(), sv.end()) == "hello");

		auto sv2 = plg::vector<uint64_t>{std::initializer_list<uint64_t>{1, 2, 3, 4, 5, 6, 7, 99999999}};
		REQUIRE(sv2.size() == 8);
		REQUIRE(sv2.back() == 99999999);
		REQUIRE(sv2.front() == 1);
	}

}
