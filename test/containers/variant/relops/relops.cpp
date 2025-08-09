#include <catch_amalgamated.hpp>

#include <utility>

#include <plg/variant.hpp>

namespace {
#if TEST_HAS_NO_EXCEPTIONS
	struct MakeEmptyT {
		MakeEmptyT() = default;
		MakeEmptyT(MakeEmptyT&&) { throw 42; }
		MakeEmptyT& operator=(MakeEmptyT&&) { throw 42; }
	};
	inline bool operator==(const MakeEmptyT&, const MakeEmptyT&) {
		REQUIRE(false);
		return false;
	}
	inline bool operator!=(const MakeEmptyT&, const MakeEmptyT&) {
		REQUIRE(false);
		return false;
	}
	inline bool operator<(const MakeEmptyT&, const MakeEmptyT&) {
		REQUIRE(false);
		return false;
	}
	inline bool operator<=(const MakeEmptyT&, const MakeEmptyT&) {
		REQUIRE(false);
		return false;
	}
	inline bool operator>(const MakeEmptyT&, const MakeEmptyT&) {
		REQUIRE(false);
		return false;
	}
	inline bool operator>=(const MakeEmptyT&, const MakeEmptyT&) {
		REQUIRE(false);
		return false;
	}

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

	struct MyBool {
		bool value;
		constexpr explicit MyBool(bool v) : value(v) {}
		constexpr operator bool() const noexcept { return value; }
	};

	struct ComparesToMyBool {
		int value = 0;
	};
	inline constexpr MyBool operator==(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
		return MyBool(LHS.value == RHS.value);
	}
	inline constexpr MyBool operator!=(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
		return MyBool(LHS.value != RHS.value);
	}
	inline constexpr MyBool operator<(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
		return MyBool(LHS.value < RHS.value);
	}
	inline constexpr MyBool operator<=(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
		return MyBool(LHS.value <= RHS.value);
	}
	inline constexpr MyBool operator>(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
		return MyBool(LHS.value > RHS.value);
	}
	inline constexpr MyBool operator>=(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
		return MyBool(LHS.value >= RHS.value);
	}

	template<class T1, class T2>
	void test_equality_basic() {
		{
			using V = plg::variant<T1, T2>;
			constexpr V v1(plg::in_place_index<0>, T1{42});
			constexpr V v2(plg::in_place_index<0>, T1{42});
			static_assert(v1 == v2);
			static_assert(v2 == v1);
			static_assert(!(v1 != v2));
			static_assert(!(v2 != v1));
		}
		{
			using V = plg::variant<T1, T2>;
			constexpr V v1(plg::in_place_index<0>, T1{42});
			constexpr V v2(plg::in_place_index<0>, T1{43});
			static_assert(!(v1 == v2));
			static_assert(!(v2 == v1));
			static_assert(v1 != v2);
			static_assert(v2 != v1);
		}
		{
			using V = plg::variant<T1, T2>;
			constexpr V v1(plg::in_place_index<0>, T1{42});
			constexpr V v2(plg::in_place_index<1>, T2{42});
			static_assert(!(v1 == v2));
			static_assert(!(v2 == v1));
			static_assert(v1 != v2);
			static_assert(v2 != v1);
		}
		{
			using V = plg::variant<T1, T2>;
			constexpr V v1(plg::in_place_index<1>, T2{42});
			constexpr V v2(plg::in_place_index<1>, T2{42});
			static_assert(v1 == v2);
			static_assert(v2 == v1);
			static_assert(!(v1 != v2));
			static_assert(!(v2 != v1));
		}
	}

	template<class Var>
	constexpr bool test_less(const Var& l, const Var& r, bool expect_less,
							 bool expect_greater) {
		static_assert(std::is_same_v<decltype(l < r), bool>);
		static_assert(std::is_same_v<decltype(l <= r), bool>);
		static_assert(std::is_same_v<decltype(l > r), bool>);
		static_assert(std::is_same_v<decltype(l >= r), bool>);

		return ((l < r) == expect_less) && (!(l >= r) == expect_less) &&
			   ((l > r) == expect_greater) && (!(l <= r) == expect_greater);
	}

	template<class T1, class T2>
	void test_relational_basic() {
		{// same index, same value
			using V = plg::variant<T1, T2>;
			constexpr V v1(plg::in_place_index<0>, T1{1});
			constexpr V v2(plg::in_place_index<0>, T1{1});
			static_assert(test_less(v1, v2, false, false));
		}
		{// same index, value < other_value
			using V = plg::variant<T1, T2>;
			constexpr V v1(plg::in_place_index<0>, T1{0});
			constexpr V v2(plg::in_place_index<0>, T1{1});
			static_assert(test_less(v1, v2, true, false));
		}
		{// same index, value > other_value
			using V = plg::variant<T1, T2>;
			constexpr V v1(plg::in_place_index<0>, T1{1});
			constexpr V v2(plg::in_place_index<0>, T1{0});
			static_assert(test_less(v1, v2, false, true));
		}
		{// LHS.index() < RHS.index()
			using V = plg::variant<T1, T2>;
			constexpr V v1(plg::in_place_index<0>, T1{0});
			constexpr V v2(plg::in_place_index<1>, T2{0});
			static_assert(test_less(v1, v2, true, false));
		}
		{// LHS.index() > RHS.index()
			using V = plg::variant<T1, T2>;
			constexpr V v1(plg::in_place_index<1>, T2{0});
			constexpr V v2(plg::in_place_index<0>, T1{0});
			static_assert(test_less(v1, v2, false, true));
		}
	}
} // namespace

TEST_CASE("variant operator > operators", "[variant]") {
	SECTION("operator==(variant<Types...> const&, variant<Types...> const&) noexcept > basic") {
		test_relational_basic<int, long>();
		test_relational_basic<ComparesToMyBool, int>();
		test_relational_basic<int, ComparesToMyBool>();
		test_relational_basic<ComparesToMyBool, ComparesToMyBool>();
#if TEST_HAS_NO_EXCEPTIONS
		{// LHS.index() < RHS.index(), RHS is empty
			using V = plg::variant<int, MakeEmptyT>;
			V v1;
			V v2;
			makeEmpty(v2);
			REQUIRE(test_less(v1, v2, false, true));
		}
		{// LHS.index() > RHS.index(), LHS is empty
			using V = plg::variant<int, MakeEmptyT>;
			V v1;
			makeEmpty(v1);
			V v2;
			REQUIRE(test_less(v1, v2, true, false));
		}
		{// LHS.index() == RHS.index(), LHS and RHS are empty
			using V = plg::variant<int, MakeEmptyT>;
			V v1;
			makeEmpty(v1);
			V v2;
			makeEmpty(v2);
			REQUIRE(test_less(v1, v2, false, false));
		}
#endif
	}
#if TEST_HAS_NO_EXCEPTIONS
	SECTION("operator==(variant<Types...> const&, variant<Types...> const&) noexcept > equality") {
		test_equality_basic<int, long>();
		test_equality_basic<ComparesToMyBool, int>();
		test_equality_basic<int, ComparesToMyBool>();
		test_equality_basic<ComparesToMyBool, ComparesToMyBool>();
		{
			using V = plg::variant<int, MakeEmptyT>;
			V v1;
			V v2;
			makeEmpty(v2);
			REQUIRE(!(v1 == v2));
			REQUIRE(!(v2 == v1));
			REQUIRE(v1 != v2);
			REQUIRE(v2 != v1);
		}
		{
			using V = plg::variant<int, MakeEmptyT>;
			V v1;
			makeEmpty(v1);
			V v2;
			REQUIRE(!(v1 == v2));
			REQUIRE(!(v2 == v1));
			REQUIRE(v1 != v2);
			REQUIRE(v2 != v1);
		}
		{
			using V = plg::variant<int, MakeEmptyT>;
			V v1;
			makeEmpty(v1);
			V v2;
			makeEmpty(v2);
			REQUIRE(v1 == v2);
			REQUIRE(v2 == v1);
			REQUIRE(!(v1 != v2));
			REQUIRE(!(v2 != v1));
		}
	}
#endif
}

