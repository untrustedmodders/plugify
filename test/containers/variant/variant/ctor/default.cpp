#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

namespace {
	struct NonDefaultConstructible {
		constexpr NonDefaultConstructible(int) {}
	};

	struct NotNoexcept {
		NotNoexcept() noexcept(false) {}
	};

#if TEST_HAS_NO_EXCEPTIONS
	struct DefaultCtorThrows {
		DefaultCtorThrows() { throw 42; }
	};
#endif

} // namespace

TEST_CASE("variant constructor", "[variant]") {
	SECTION("variant() noexcept(see below) > basic") {
		{
			plg::variant<int> v;
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v) == 0);
		}
		{
			plg::variant<int, long> v;
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v) == 0);
		}
		{
			plg::variant<int, NonDefaultConstructible> v;
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v) == 0);
		}
		{
			using V = plg::variant<int, long>;
			constexpr V v;
			static_assert(v.index() == 0);
			static_assert(plg::get<0>(v) == 0);
		}
		{
			using V = plg::variant<int, long>;
			constexpr V v;
			static_assert(v.index() == 0);
			static_assert(plg::get<0>(v) == 0);
		}
		{
			using V = plg::variant<int, NonDefaultConstructible>;
			constexpr V v;
			static_assert(v.index() == 0);
			static_assert(plg::get<0>(v) == 0);
		}
	}
	SECTION("variant() noexcept(see below) > sfinae") {
		{
			using V = plg::variant<plg::monostate, int>;
			static_assert(std::is_default_constructible<V>::value);
		}
		{
			using V = plg::variant<NonDefaultConstructible, int>;
			static_assert(!std::is_default_constructible<V>::value);
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int&, int>;
			static_assert(!std::is_default_constructible<V>::value);
		}
#endif
	}
	SECTION("variant() noexcept(see below) > noexcept") {
		{
			using V = plg::variant<int>;
			static_assert(std::is_nothrow_default_constructible<V>::value);
		}
		{
			using V = plg::variant<NotNoexcept>;
			static_assert(!std::is_nothrow_default_constructible<V>::value);
		}
	}
#if TEST_HAS_NO_EXCEPTIONS
	SECTION("variant() noexcept(see below) > throws") {
		using V = plg::variant<DefaultCtorThrows, int>;
		try {
			V v;
			REQUIRE(false);
		} catch (const int& ex) {
			REQUIRE(ex == 42);
		} catch (...) {
			REQUIRE(false);
		}
	}
#endif
}
