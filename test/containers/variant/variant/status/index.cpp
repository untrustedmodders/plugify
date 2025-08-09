#include <catch_amalgamated.hpp>

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
}// namespace

TEST_CASE("variant > index", "[variant]") {

	SECTION("size_t index() const noexcept") {
		{
			using V = plg::variant<int, long>;
			constexpr V v;
			static_assert(v.index() == 0);
		}
		{
			using V = plg::variant<int, long>;
			V v;
			REQUIRE(v.index() == 0);
		}
		{
			using V = plg::variant<int, long>;
			constexpr V v(plg::in_place_index<1>);
			static_assert(v.index() == 1);
		}
		{
			using V = plg::variant<int, std::string>;
			V v("abc");
			REQUIRE(v.index() == 1);
			v = 42;
			REQUIRE(v.index() == 0);
		}
#if TEST_HAS_NO_EXCEPTIONS
		{
			using V = plg::variant<int, MakeEmptyT>;
			V v;
			REQUIRE(v.index() == 0);
			makeEmpty(v);
			REQUIRE(v.index() == plg::variant_npos);
		}
#endif
	}
}
