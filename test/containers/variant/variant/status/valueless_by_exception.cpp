#include <catch_amalgamated.hpp>

#include <plugify/variant.hpp>

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
}// namespace

TEST_CASE("variant > valueless_by_exception", "[variant]") {
	SECTION("bool valueless_by_exception() const noexcept") {
		{
			using V = plg::variant<int, long>;
			constexpr V v;
			static_assert(!v.valueless_by_exception());
		}
		{
			using V = plg::variant<int, long>;
			V v;
			REQUIRE(!v.valueless_by_exception());
		}
		{
			using V = plg::variant<int, long, std::string>;
			const V v("abc");
			REQUIRE(!v.valueless_by_exception());
		}
#if TEST_HAS_NO_EXCEPTIONS
		{
			using V = plg::variant<int, MakeEmptyT>;
			V v;
			REQUIRE(!v.valueless_by_exception());
			makeEmpty(v);
			REQUIRE(v.valueless_by_exception());
		}
#endif
	}
}
