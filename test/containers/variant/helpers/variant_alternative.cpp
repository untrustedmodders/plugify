#include <catch_amalgamated.hpp>

#include <plugifyvariant.hpp>

namespace {
	template<class V, size_t I, class E>
	void test() {
		static_assert(std::is_same_v<typename plg::variant_alternative<I, V>::type, E>);
		static_assert(std::is_same_v<typename plg::variant_alternative<I, const V>::type, const E>);
		static_assert(std::is_same_v<typename plg::variant_alternative<I, volatile V>::type, volatile E>);
		static_assert(std::is_same_v<typename plg::variant_alternative<I, const volatile V>::type, const volatile E>);
		static_assert(std::is_same_v<plg::variant_alternative_t<I, V>, E>);
		static_assert(std::is_same_v<plg::variant_alternative_t<I, const V>, const E>);
		static_assert(std::is_same_v<plg::variant_alternative_t<I, volatile V>, volatile E>);
		static_assert(std::is_same_v<plg::variant_alternative_t<I, const volatile V>, const volatile E>);
	}
} // namespace

TEST_CASE("variant_alternative", "[variant]") {
	SECTION("struct variant_alternative<I, variant<Types...>>") {
		using V = plg::variant<int, void*, const void*, long double>;
		test<V, 0, int>();
		test<V, 1, void*>();
		test<V, 2, const void*>();
		test<V, 3, long double>();
	}
}