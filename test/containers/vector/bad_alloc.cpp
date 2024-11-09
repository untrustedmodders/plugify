#include <catch_amalgamated.hpp>

#include <plugify/vector.hpp>

// make sure all this works even when valgrind is not installed
#if __has_include(<valgrind/valgrind.h>)
#	include <valgrind/valgrind.h>
#else
#	ifndef RUNNING_ON_VALGRIND
#		define RUNNING_ON_VALGRIND 0
#	endif
#endif

#include <limits>
#include <stdexcept>
#include <vector>

#define SANITIZER_ACTIVE 0

#if defined(__has_feature)
#	if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
#		undef SANITIZER_ACTIVE
#		define SANITIZER_ACTIVE 1
#	endif
#endif

TEST_CASE("allocator", "[vector]") {
	SECTION("reserve_bad_alloc") {
		if constexpr (RUNNING_ON_VALGRIND || SANITIZER_ACTIVE) {
			// this test doesn't work with valgrind or some sanitizers.
		} else {
			auto sv = plg::vector<std::string>();
			//REQUIRE(sv.max_size() == 0x7ffffffffffffff);
			REQUIRE_THROWS_AS(sv.reserve(sv.max_size()), std::bad_alloc);
		}
	}
}
