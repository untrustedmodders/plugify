#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

#include "app/variant_tester.hpp"

TEST_CASE("variant operator > assignment > simple", "[variant]") {
	SECTION("variant& operator=(T&&) noexcept(see below)") {
		static_assert(!std::is_assignable<plg::variant<int, int>, int>::value);
		static_assert(!std::is_assignable<plg::variant<long, long long>, int>::value);
		static_assert(std::is_assignable<plg::variant<char>, int>::value == VariantAllowsNarrowingConversions);

		static_assert(std::is_assignable<plg::variant<std::string, float>, int>::value == VariantAllowsNarrowingConversions);
		static_assert(std::is_assignable<plg::variant<std::string, double>, int>::value == VariantAllowsNarrowingConversions);
		static_assert(!std::is_assignable<plg::variant<std::string, bool>, int>::value);

		static_assert(!std::is_assignable<plg::variant<int, bool>, decltype("meow")>::value);
		static_assert(!std::is_assignable<plg::variant<int, const bool>, decltype("meow")>::value);
		static_assert(!std::is_assignable<plg::variant<int, const volatile bool>, decltype("meow")>::value);

		// libc++ implementation completely forbids any implicit conversion to bool, not plg::variant
		// static_assert(!std::is_assignable<plg::variant<bool>, std::true_type>::value);
		static_assert(!std::is_assignable<plg::variant<bool>, std::unique_ptr<char>>::value);
		static_assert(!std::is_assignable<plg::variant<bool>, decltype(nullptr)>::value);
	}
}