#include <catch_amalgamated.hpp>

#include <plugify/variant.hpp>

namespace {
	template<class V, size_t E>
	void test() {
		static_assert(plg::variant_size<V>::value == E);
		static_assert(plg::variant_size<const V>::value == E);
		static_assert(plg::variant_size<volatile V>::value == E);
		static_assert(plg::variant_size<const volatile V>::value == E);
		static_assert(plg::variant_size_v<V> == E);
		static_assert(plg::variant_size_v<const V> == E);
		static_assert(plg::variant_size_v<volatile V> == E);
		static_assert(plg::variant_size_v<const volatile V> == E);
		static_assert(std::is_base_of<std::integral_constant<std::size_t, E>, plg::variant_size<V>>::value);
	}
} // namespace

TEST_CASE("variant_size", "[variant]") {
	SECTION("size_t variant_size_v = variant_size<T>::value") {

		// PLG : this is ill-formed, why is this in the test suite?
		// test<plg::variant<>, 0>();
		test<plg::variant<void*>, 1>();
		test<plg::variant<long, long, void*, double>, 4>();
	}
}
