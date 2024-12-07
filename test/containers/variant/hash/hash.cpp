#include <catch_amalgamated.hpp>

#include <plugifyvariant.hpp>

namespace {
#if TEST_HAS_NO_EXCEPTIONS
	struct MakeEmptyT {
		static int alive;
		MakeEmptyT() { ++alive; }
		MakeEmptyT(const MakeEmptyT&) {
			++alive;
			// Don't throw from the copy constructor since variant's assignment
			// operator performs a copy before committing to the assignment.
		}
		MakeEmptyT(MakeEmptyT&&) { throw 42; }
		MakeEmptyT& operator=(const MakeEmptyT&) { throw 42; }
		MakeEmptyT& operator=(MakeEmptyT&&) { throw 42; }
		~MakeEmptyT() { --alive; }
	};

	int MakeEmptyT::alive = 0;

	template<class Variant>
	void makeEmpty(Variant& v) {
		Variant v2(plg::in_place_type<MakeEmptyT>);
		try {
			v = std::move(v2);
			REQUIRE(false);
		} catch (...) {
			REQUIRE(v.valueless_by_exception());
		}
	}
#endif
} // namespace

#if TEST_HAS_NO_EXCEPTIONS
namespace std {
	template<>
	struct hash<::MakeEmptyT> {
		size_t operator()(const ::MakeEmptyT&) const {
			REQUIRE(false);
			return 0;
		}
	};
}// namespace std
#endif

struct A {};
struct B {};

namespace std {
	template<>
	struct hash<B> {
		size_t operator()(B const&) const {
			return 0;
		}
	};
}// namespace std

TEST_CASE("variant has", "[variant]") {
	SECTION("struct hash<monostate> > variant") {
		{
			using V = plg::variant<int, long, int>;
			using H = std::hash<V>;
			const V v(plg::in_place_index<0>, 42);
			const V v_copy = v;
			V v2(plg::in_place_index<0>, 100);
			const H h{};
			REQUIRE(h(v) == h(v));
			REQUIRE(h(v) != h(v2));
			REQUIRE(h(v) == h(v_copy));
			{
				static_assert(std::is_same_v<decltype(h(v)), std::size_t>);
				static_assert(std::is_copy_constructible<H>::value);
			}
		}
		{
			using V = plg::variant<plg::monostate, int, long, const char*>;
			using H = std::hash<V>;
			const char* str = "hello";
			const V v0;
			const V v0_other;
			const V v1(42);
			const V v1_other(100);
			V v2(100l);
			V v2_other(999l);
			V v3(str);
			V v3_other("not hello");
			const H h{};
			REQUIRE(h(v0) == h(v0));
			REQUIRE(h(v0) == h(v0_other));
			REQUIRE(h(v1) == h(v1));
			REQUIRE(h(v1) != h(v1_other));
			REQUIRE(h(v2) == h(v2));
			REQUIRE(h(v2) != h(v2_other));
			REQUIRE(h(v3) == h(v3));
			REQUIRE(h(v3) != h(v3_other));
			REQUIRE(h(v0) != h(v1));
			REQUIRE(h(v0) != h(v2));
			REQUIRE(h(v0) != h(v3));
			REQUIRE(h(v1) != h(v2));
			REQUIRE(h(v1) != h(v3));
			REQUIRE(h(v2) != h(v3));
		}
#if TEST_HAS_NO_EXCEPTIONS
		{
			using V = plg::variant<int, MakeEmptyT>;
			using H = std::hash<V>;
			V v;
			makeEmpty(v);
			V v2;
			makeEmpty(v2);
			const H h{};
			REQUIRE(h(v) == h(v2));
		}
#endif
	}
	SECTION("struct hash<monostate> > duplicate_elements") {
		// Test that the index of the alternative participates in the hash value.
		using V = plg::variant<plg::monostate, plg::monostate>;
		using H = std::hash<V>;
		H h{};
		const V v1(plg::in_place_index<0>);
		const V v2(plg::in_place_index<1>);
		REQUIRE(h(v1) == h(v1));
		REQUIRE(h(v2) == h(v2));
		REQUIRE(h(v1) != h(v2));
	}
	SECTION("struct hash<monostate> > monostate") {
		using H = std::hash<plg::monostate>;
		const H h{};
		plg::monostate m1{};
		const plg::monostate m2{};
		REQUIRE(h(m1) == h(m1));
		REQUIRE(h(m2) == h(m2));
		REQUIRE(h(m1) == h(m2));
		{
			static_assert(std::is_same_v<decltype(h(m1)), std::size_t>);
			REQUIRE(h(m1));
			static_assert(std::is_copy_constructible<H>::value);
		}
		{
			//test_hash_enabled_for_type<plg::monostate>();
		}
	}
	/*SECTION("truct hash<monostate> > variant_enabled") {
		{
			test_hash_enabled_for_type<plg::variant<int>>();
			test_hash_enabled_for_type<plg::variant<int*, long, double, const int>>();
		}
		{
			test_hash_disabled_for_type<plg::variant<int, A>>();
			test_hash_disabled_for_type<plg::variant<const A, void*>>();
		}
		{
			test_hash_enabled_for_type<plg::variant<int, B>>();
			test_hash_enabled_for_type<plg::variant<const B, int>>();
		}
	}*/
}
