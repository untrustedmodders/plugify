#include <catch_amalgamated.hpp>

#include <plugifyvariant.hpp>

#if TEST_HAS_NO_EXCEPTIONS
namespace {
	struct test {
		explicit operator int() {
			throw std::exception();
		}
	};
}

TEST_CASE("throw", "[variant]") {
	SECTION("throwing_conversion") {
		plg::variant<int, bool> v{true};

		test t;

		try {
			v.emplace<0>(t);
		} catch (...) {}

		assert(v.index() == 1);
		assert(get<1>(v) == true);
	}
}
#endif