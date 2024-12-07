#include <catch_amalgamated.hpp>

#include <plugify/variant.hpp>

namespace {

	struct NonT {
		NonT(int v) : value(v) {}
		NonT(const NonT& o) : value(o.value) {}
		int value;
	};
	static_assert(!std::is_trivially_copy_constructible<NonT>::value);

	struct NoCopy {
		NoCopy(const NoCopy&) = delete;
	};

	struct MoveOnly {
		MoveOnly(const MoveOnly&) = delete;
		MoveOnly(MoveOnly&&) = default;
	};

	struct MoveOnlyNT {
		MoveOnlyNT(const MoveOnlyNT&) = delete;
		MoveOnlyNT(MoveOnlyNT&&) {}
	};

	struct NTCopy {
		constexpr NTCopy(int v) : value(v) {}
		NTCopy(const NTCopy& that) : value(that.value) {}
		NTCopy(NTCopy&&) = delete;
		int value;
	};

	static_assert(!std::is_trivially_copy_constructible<NTCopy>::value);
	static_assert(std::is_copy_constructible<NTCopy>::value);

	struct TCopy {
		constexpr TCopy(int v) : value(v) {}
		TCopy(TCopy const&) = default;
		TCopy(TCopy&&) = delete;
		int value;
	};

	static_assert(std::is_trivially_copy_constructible<TCopy>::value);

	struct TCopyNTMove {
		constexpr TCopyNTMove(int v) : value(v) {}
		TCopyNTMove(const TCopyNTMove&) = default;
		TCopyNTMove(TCopyNTMove&& that) : value(that.value) { that.value = -1; }
		int value;
	};

	static_assert(std::is_trivially_copy_constructible<TCopyNTMove>::value);

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

	template<size_t Idx>
	constexpr bool test_constexpr_copy_ctor_imp(plg::variant<long, void*, const int> const& v) {
		auto v2 = v;
		return v2.index() == v.index() &&
			   v2.index() == Idx &&
			   plg::get<Idx>(v2) == plg::get<Idx>(v);
	}
} // namespace

TEST_CASE("variant copy constructor", "[variant]") {
	SECTION("variant(variant const&) > basic") {
		{
			plg::variant<int> v(plg::in_place_index<0>, 42);
			plg::variant<int> v2 = v;
			REQUIRE(v2.index() == 0);
			REQUIRE(plg::get<0>(v2) == 42);
		}
		{
			plg::variant<int, long> v(plg::in_place_index<1>, 42);
			plg::variant<int, long> v2 = v;
			REQUIRE(v2.index() == 1);
			REQUIRE(plg::get<1>(v2) == 42);
		}
		{
			plg::variant<NonT> v(plg::in_place_index<0>, 42);
			REQUIRE(v.index() == 0);
			plg::variant<NonT> v2(v);
			REQUIRE(v2.index() == 0);
			REQUIRE(plg::get<0>(v2).value == 42);
		}
		{
			plg::variant<int, NonT> v(plg::in_place_index<1>, 42);
			REQUIRE(v.index() == 1);
			plg::variant<int, NonT> v2(v);
			REQUIRE(v2.index() == 1);
			REQUIRE(plg::get<1>(v2).value == 42);
		}

		// Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
		{
			constexpr plg::variant<int> v(plg::in_place_index<0>, 42);
			static_assert(v.index() == 0);
			constexpr plg::variant<int> v2 = v;
			static_assert(v2.index() == 0);
			static_assert(plg::get<0>(v2) == 42);
		}
		{
			constexpr plg::variant<int, long> v(plg::in_place_index<1>, 42);
			static_assert(v.index() == 1);
			constexpr plg::variant<int, long> v2 = v;
			static_assert(v2.index() == 1);
			static_assert(plg::get<1>(v2) == 42);
		}
		{
			constexpr plg::variant<TCopy> v(plg::in_place_index<0>, 42);
			static_assert(v.index() == 0);
			constexpr plg::variant<TCopy> v2(v);
			static_assert(v2.index() == 0);
			static_assert(plg::get<0>(v2).value == 42);
		}
		{
			constexpr plg::variant<int, TCopy> v(plg::in_place_index<1>, 42);
			static_assert(v.index() == 1);
			constexpr plg::variant<int, TCopy> v2(v);
			static_assert(v2.index() == 1);
			static_assert(plg::get<1>(v2).value == 42);
		}
		{
			constexpr plg::variant<TCopyNTMove> v(plg::in_place_index<0>, 42);
			static_assert(v.index() == 0);
			constexpr plg::variant<TCopyNTMove> v2(v);
			static_assert(v2.index() == 0);
			static_assert(plg::get<0>(v2).value == 42);
		}
		{
			constexpr plg::variant<int, TCopyNTMove> v(plg::in_place_index<1>, 42);
			static_assert(v.index() == 1);
			constexpr plg::variant<int, TCopyNTMove> v2(v);
			static_assert(v2.index() == 1);
			static_assert(plg::get<1>(v2).value == 42);
		}
	}
#if TEST_HAS_NO_EXCEPTIONS
	SECTION("variant(variant const&) > valueless_by_exception") {
		using V = plg::variant<int, MakeEmptyT>;
		V v1;
		makeEmpty(v1);
		const V& cv1 = v1;
		V v(cv1);
		REQUIRE(v.valueless_by_exception());
	}
#endif
	SECTION("variant(variant const&) > sfinae") {
		{
			using V = plg::variant<int, long>;
			static_assert(std::is_copy_constructible<V>::value);
		}
		{
			using V = plg::variant<int, NoCopy>;
			static_assert(!std::is_copy_constructible<V>::value);
		}
		{
			using V = plg::variant<int, MoveOnly>;
			static_assert(!std::is_copy_constructible<V>::value);
		}
		{
			using V = plg::variant<int, MoveOnlyNT>;
			static_assert(!std::is_copy_constructible<V>::value);
		}

		// Make sure we properly propagate triviality (see P0602R4).
		{
			using V = plg::variant<int, long>;
			static_assert(std::is_trivially_copy_constructible<V>::value);
		}
		{
			using V = plg::variant<int, NTCopy>;
			static_assert(!std::is_trivially_copy_constructible<V>::value);
			static_assert(std::is_copy_constructible<V>::value);
		}
		{
			using V = plg::variant<int, TCopy>;
			static_assert(std::is_trivially_copy_constructible<V>::value);
		}
		{
			using V = plg::variant<int, TCopyNTMove>;
			static_assert(std::is_trivially_copy_constructible<V>::value);
		}
	}
	SECTION("variant(variant const&) > copy_ctor") {
		// Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
		using V = plg::variant<long, void*, const int>;
		static_assert(std::is_trivially_destructible<V>::value);
		static_assert(std::is_trivially_copy_constructible<V>::value);
		static_assert(std::is_trivially_move_constructible<V>::value);
		//static_assert(!std::is_copy_assignable<V>::value);
		//static_assert(!std::is_move_assignable<V>::value);
		static_assert(test_constexpr_copy_ctor_imp<0>(V(42l)));
		static_assert(test_constexpr_copy_ctor_imp<1>(V(nullptr)));
		static_assert(test_constexpr_copy_ctor_imp<2>(V(101)));
	}
}
