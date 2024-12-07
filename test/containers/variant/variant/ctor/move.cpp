#include <catch_amalgamated.hpp>

#include <plugify/variant.hpp>

namespace {
	struct ThrowsMove {
		ThrowsMove(ThrowsMove&&) noexcept(false) {}
	};

	struct NoCopy {
		NoCopy(const NoCopy&) = delete;
	};

	struct MoveOnly {
		int value;
		MoveOnly(int v) : value(v) {}
		MoveOnly(const MoveOnly&) = delete;
		MoveOnly(MoveOnly&&) = default;
	};

	struct MoveOnlyNT {
		int value;
		MoveOnlyNT(int v) : value(v) {}
		MoveOnlyNT(const MoveOnlyNT&) = delete;
		MoveOnlyNT(MoveOnlyNT&& other) : value(other.value) { other.value = -1; }
	};

	struct NTMove {
		constexpr NTMove(int v) : value(v) {}
		NTMove(const NTMove&) = delete;
		NTMove(NTMove&& that) : value(that.value) { that.value = -1; }
		int value;
	};

	static_assert(!std::is_trivially_move_constructible<NTMove>::value);
	static_assert(std::is_move_constructible<NTMove>::value);

	struct TMove {
		constexpr TMove(int v) : value(v) {}
		TMove(const TMove&) = delete;
		TMove(TMove&&) = default;
		int value;
	};

	static_assert(std::is_trivially_move_constructible<TMove>::value);

	struct TMoveNTCopy {
		constexpr TMoveNTCopy(int v) : value(v) {}
		TMoveNTCopy(const TMoveNTCopy& that) : value(that.value) {}
		TMoveNTCopy(TMoveNTCopy&&) = default;
		int value;
	};

	static_assert(std::is_trivially_move_constructible<TMoveNTCopy>::value);

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

	template<typename T>
	struct Result {
		size_t index;
		T value;
	};

	template<size_t Idx>
	constexpr bool test_constexpr_ctor_imp(plg::variant<long, void*, const int> const& v) {
		auto copy = v;
		auto v2 = std::move(copy);
		return v2.index() == v.index() &&
			   v2.index() == Idx &&
			   plg::get<Idx>(v2) == plg::get<Idx>(v);
	}
} // namespace

TEST_CASE("variant move constructor", "[variant]") {
	SECTION("variant(variant&&) noexcept(see below) > ctor_basic") {
		{
			plg::variant<int> v(plg::in_place_index<0>, 42);
			plg::variant<int> v2 = std::move(v);
			REQUIRE(v2.index() == 0);
			REQUIRE(plg::get<0>(v2) == 42);
		}
		{
			plg::variant<int, long> v(plg::in_place_index<1>, 42);
			plg::variant<int, long> v2 = std::move(v);
			REQUIRE(v2.index() == 1);
			REQUIRE(plg::get<1>(v2) == 42);
		}
		{
			plg::variant<MoveOnly> v(plg::in_place_index<0>, 42);
			REQUIRE(v.index() == 0);
			plg::variant<MoveOnly> v2(std::move(v));
			REQUIRE(v2.index() == 0);
			REQUIRE(plg::get<0>(v2).value == 42);
		}
		{
			plg::variant<int, MoveOnly> v(plg::in_place_index<1>, 42);
			REQUIRE(v.index() == 1);
			plg::variant<int, MoveOnly> v2(std::move(v));
			REQUIRE(v2.index() == 1);
			REQUIRE(plg::get<1>(v2).value == 42);
		}
		{
			plg::variant<MoveOnlyNT> v(plg::in_place_index<0>, 42);
			REQUIRE(v.index() == 0);
			plg::variant<MoveOnlyNT> v2(std::move(v));
			REQUIRE(v2.index() == 0);
			REQUIRE(plg::get<0>(v).value == -1);
			REQUIRE(plg::get<0>(v2).value == 42);
		}
		{
			plg::variant<int, MoveOnlyNT> v(plg::in_place_index<1>, 42);
			REQUIRE(v.index() == 1);
			plg::variant<int, MoveOnlyNT> v2(std::move(v));
			REQUIRE(v2.index() == 1);
			REQUIRE(plg::get<1>(v).value == -1);
			REQUIRE(plg::get<1>(v2).value == 42);
		}

		// Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
		{
			struct {
				constexpr Result<int> operator()() const {
					plg::variant<int> v(plg::in_place_index<0>, 42);
					plg::variant<int> v2 = std::move(v);
					return {v2.index(), plg::get<0>(std::move(v2))};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 0);
			static_assert(result.value == 42);
		}
		{
			struct {
				constexpr Result<long> operator()() const {
					plg::variant<int, long> v(plg::in_place_index<1>, 42);
					plg::variant<int, long> v2 = std::move(v);
					return {v2.index(), plg::get<1>(std::move(v2))};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value == 42);
		}
		{
			struct {
				constexpr Result<TMove> operator()() const {
					plg::variant<TMove> v(plg::in_place_index<0>, 42);
					plg::variant<TMove> v2(std::move(v));
					return {v2.index(), plg::get<0>(std::move(v2))};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 0);
			static_assert(result.value.value == 42);
		}
		{
			struct {
				constexpr Result<TMove> operator()() const {
					plg::variant<int, TMove> v(plg::in_place_index<1>, 42);
					plg::variant<int, TMove> v2(std::move(v));
					return {v2.index(), plg::get<1>(std::move(v2))};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value.value == 42);
		}
		{
			struct {
				constexpr Result<TMoveNTCopy> operator()() const {
					plg::variant<TMoveNTCopy> v(plg::in_place_index<0>, 42);
					plg::variant<TMoveNTCopy> v2(std::move(v));
					return {v2.index(), plg::get<0>(std::move(v2))};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 0);
			static_assert(result.value.value == 42);
		}
		{
			struct {
				constexpr Result<TMoveNTCopy> operator()() const {
					plg::variant<int, TMoveNTCopy> v(plg::in_place_index<1>, 42);
					plg::variant<int, TMoveNTCopy> v2(std::move(v));
					return {v2.index(), plg::get<1>(std::move(v2))};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value.value == 42);
		}
	}
#if TEST_HAS_NO_EXCEPTIONS
	SECTION("variant(variant&&) noexcept(see below) > ctor_valueless_by_exception") {
		using V = plg::variant<int, MakeEmptyT>;
		V v1;
		makeEmpty(v1);
		V v(std::move(v1));
		REQUIRE(v.valueless_by_exception());
	}
#endif
	SECTION("variant(variant&&) noexcept(see below) > noexcept") {
		{
			using V = plg::variant<int, long>;
			static_assert(std::is_nothrow_move_constructible<V>::value);
		}
		{
			using V = plg::variant<int, MoveOnly>;
			static_assert(std::is_nothrow_move_constructible<V>::value);
		}
		{
			using V = plg::variant<int, MoveOnlyNT>;
			static_assert(!std::is_nothrow_move_constructible<V>::value);
		}
		{
			using V = plg::variant<int, ThrowsMove>;
			static_assert(!std::is_nothrow_move_constructible<V>::value);
		}
	}
	SECTION("variant(variant&&) noexcept(see below) > ctor_sfinae") {
		{
			using V = plg::variant<int, long>;
			static_assert(std::is_move_constructible<V>::value);
		}
		{
			using V = plg::variant<int, MoveOnly>;
			static_assert(std::is_move_constructible<V>::value);
		}
		{
			using V = plg::variant<int, MoveOnlyNT>;
			static_assert(std::is_move_constructible<V>::value);
		}
		{
			using V = plg::variant<int, NoCopy>;
			static_assert(!std::is_move_constructible<V>::value);
		}

		// Make sure we properly propagate triviality (see P0602R4).
		{
			using V = plg::variant<int, long>;
			static_assert(std::is_trivially_move_constructible<V>::value);
		}
		{
			using V = plg::variant<int, NTMove>;
			static_assert(!std::is_trivially_move_constructible<V>::value);
			static_assert(std::is_move_constructible<V>::value);
		}
		{
			using V = plg::variant<int, TMove>;
			static_assert(std::is_trivially_move_constructible<V>::value);
		}
		{
			using V = plg::variant<int, TMoveNTCopy>;
			static_assert(std::is_trivially_move_constructible<V>::value);
		}
	}
	SECTION("variant(variant&&) noexcept(see below) > move_ctor") {
		// Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
		using V = plg::variant<long, void*, const int>;
		static_assert(std::is_trivially_destructible<V>::value);
		static_assert(std::is_trivially_copy_constructible<V>::value);
		static_assert(std::is_trivially_move_constructible<V>::value);
		//static_assert(!std::is_copy_assignable<V>::value);
		//static_assert(!std::is_move_assignable<V>::value);
		static_assert(std::is_trivially_move_constructible<V>::value);
		static_assert(test_constexpr_ctor_imp<0>(V(42l)));
		static_assert(test_constexpr_ctor_imp<1>(V(nullptr)));
		static_assert(test_constexpr_ctor_imp<2>(V(101)));
	}
}
