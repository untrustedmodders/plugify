#include <catch_amalgamated.hpp>

#include <plugify/variant.hpp>

namespace {
	struct NoCopy {
		NoCopy(const NoCopy&) = delete;
		NoCopy& operator=(const NoCopy&) = default;
	};

	struct CopyOnly {
		CopyOnly(const CopyOnly&) = default;
		CopyOnly(CopyOnly&&) = delete;
		CopyOnly& operator=(const CopyOnly&) = default;
		CopyOnly& operator=(CopyOnly&&) = delete;
	};

	struct MoveOnly {
		MoveOnly(const MoveOnly&) = delete;
		MoveOnly(MoveOnly&&) = default;
		MoveOnly& operator=(const MoveOnly&) = default;
	};

	struct MoveOnlyNT {
		MoveOnlyNT(const MoveOnlyNT&) = delete;
		MoveOnlyNT(MoveOnlyNT&&) {}
		MoveOnlyNT& operator=(const MoveOnlyNT&) = default;
	};

	struct CopyAssign {
		static int alive;
		static int copy_construct;
		static int copy_assign;
		static int move_construct;
		static int move_assign;
		static void reset() {
			copy_construct = copy_assign = move_construct = move_assign = alive = 0;
		}
		CopyAssign(int v) : value(v) { ++alive; }
		CopyAssign(const CopyAssign& o) : value(o.value) {
			++alive;
			++copy_construct;
		}
		CopyAssign(CopyAssign&& o) noexcept : value(o.value) {
			o.value = -1;
			++alive;
			++move_construct;
		}
		CopyAssign& operator=(const CopyAssign& o) {
			value = o.value;
			++copy_assign;
			return *this;
		}
		CopyAssign& operator=(CopyAssign&& o) noexcept {
			value = o.value;
			o.value = -1;
			++move_assign;
			return *this;
		}
		~CopyAssign() { --alive; }
		int value;
	};

	int CopyAssign::alive = 0;
	int CopyAssign::copy_construct = 0;
	int CopyAssign::copy_assign = 0;
	int CopyAssign::move_construct = 0;
	int CopyAssign::move_assign = 0;

	struct CopyMaybeThrows {
		CopyMaybeThrows(const CopyMaybeThrows&);
		CopyMaybeThrows& operator=(const CopyMaybeThrows&);
	};
	struct CopyDoesThrow {
		CopyDoesThrow(const CopyDoesThrow&) noexcept(false);
		CopyDoesThrow& operator=(const CopyDoesThrow&) noexcept(false);
	};

	struct NTCopyAssign {
		constexpr NTCopyAssign(int v) : value(v) {}
		NTCopyAssign(const NTCopyAssign&) = default;
		NTCopyAssign(NTCopyAssign&&) = default;
		NTCopyAssign& operator=(const NTCopyAssign& that) {
			value = that.value;
			return *this;
		};
		NTCopyAssign& operator=(NTCopyAssign&&) = delete;
		int value;
	};

	static_assert(!std::is_trivially_copy_assignable<NTCopyAssign>::value);
	static_assert(std::is_copy_assignable<NTCopyAssign>::value);

	struct TCopyAssign {
		constexpr TCopyAssign(int v) : value(v) {}
		TCopyAssign(const TCopyAssign&) = default;
		TCopyAssign(TCopyAssign&&) = default;
		TCopyAssign& operator=(const TCopyAssign&) = default;
		TCopyAssign& operator=(TCopyAssign&&) = delete;
		int value;
	};

	static_assert(std::is_trivially_copy_assignable<TCopyAssign>::value);

	struct TCopyAssignNTMoveAssign {
		constexpr TCopyAssignNTMoveAssign(int v) : value(v) {}
		TCopyAssignNTMoveAssign(const TCopyAssignNTMoveAssign&) = default;
		TCopyAssignNTMoveAssign(TCopyAssignNTMoveAssign&&) = default;
		TCopyAssignNTMoveAssign& operator=(const TCopyAssignNTMoveAssign&) = default;
		TCopyAssignNTMoveAssign& operator=(TCopyAssignNTMoveAssign&& that) {
			value = that.value;
			that.value = -1;
			return *this;
		}
		int value;
	};

	static_assert(std::is_trivially_copy_assignable_v<TCopyAssignNTMoveAssign>);

#if TEST_HAS_NO_EXCEPTIONS
	struct CopyThrows {
		CopyThrows() = default;
		CopyThrows(const CopyThrows&) { throw 42; }
		CopyThrows& operator=(const CopyThrows&) { throw 42; }
	};

	struct CopyCannotThrow {
		static int alive;
		CopyCannotThrow() { ++alive; }
		CopyCannotThrow(const CopyCannotThrow&) noexcept { ++alive; }
		CopyCannotThrow(CopyCannotThrow&&) noexcept { REQUIRE(false); }
		CopyCannotThrow& operator=(const CopyCannotThrow&) noexcept = default;
		CopyCannotThrow& operator=(CopyCannotThrow&&) noexcept {
			REQUIRE(false);
			return *this;
		}
	};

	int CopyCannotThrow::alive = 0;

	struct MoveThrows {
		static int alive;
		MoveThrows() { ++alive; }
		MoveThrows(const MoveThrows&) { ++alive; }
		MoveThrows(MoveThrows&&) { throw 42; }
		MoveThrows& operator=(const MoveThrows&) { return *this; }
		MoveThrows& operator=(MoveThrows&&) { throw 42; }
		~MoveThrows() { --alive; }
	};

	int MoveThrows::alive = 0;

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

	template<size_t NewIdx, class ValueType>
	constexpr bool test_constexpr_assign_imp(
			plg::variant<long, void*, int>&& v, ValueType&& new_value) {
		const plg::variant<long, void*, int> cp(
				std::forward<ValueType>(new_value));
		v = cp;
		return v.index() == NewIdx &&
			   plg::get<NewIdx>(v) == plg::get<NewIdx>(cp);
	}
} // namespace

TEST_CASE("variant operator > assignment > copy", "[variant]") {
#if TEST_HAS_NO_EXCEPTIONS
	SECTION("variant& operator=(variant const&) > empty_empty") {
		using MET = MakeEmptyT;
		{
			using V = plg::variant<int, long, MET>;
			V v1(plg::in_place_index<0>);
			makeEmpty(v1);
			V v2(plg::in_place_index<0>);
			makeEmpty(v2);
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.valueless_by_exception());
			// REQUIRE(v1.index() == plg::variant<int, float>::npos);
		}
	}
	SECTION("variant& operator=(variant const&) > non_empty_empty") {
		using MET = MakeEmptyT;
		{
			using V = plg::variant<int, MET>;
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<0>);
			makeEmpty(v2);
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.valueless_by_exception());
			// REQUIRE(v1.index() == plg::variant<int, float>::npos);
		}
		{
			using V = plg::variant<int, MET, std::string>;
			V v1(plg::in_place_index<2>, "hello");
			V v2(plg::in_place_index<0>);
			makeEmpty(v2);
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.valueless_by_exception());
			// REQUIRE(v1.index() == plg::variant<int, float>::npos);
		}
	}
	SECTION("variant& operator=(variant const&) > empty_non_empty") {
		using MET = MakeEmptyT;
		{
			using V = plg::variant<int, MET>;
			V v1(plg::in_place_index<0>);
			makeEmpty(v1);
			V v2(plg::in_place_index<0>, 42);
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 0);
			REQUIRE(plg::get<0>(v1) == 42);
		}
		{
			using V = plg::variant<int, MET, std::string>;
			V v1(plg::in_place_index<0>);
			makeEmpty(v1);
			V v2(plg::in_place_type<std::string>, "hello");
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 2);
			REQUIRE(plg::get<2>(v1) == "hello");
		}
	}
#endif
	SECTION("variant& operator=(variant const&) > same_index") {
		{
			using V = plg::variant<int>;
			V v1(43);
			V v2(42);
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 0);
			REQUIRE(plg::get<0>(v1) == 42);
		}
		{
			using V = plg::variant<int, long, unsigned>;
			V v1(43l);
			V v2(42l);
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 1);
			REQUIRE(plg::get<1>(v1) == 42);
		}
		{
			using V = plg::variant<int, CopyAssign, unsigned>;
			V v1(plg::in_place_type<CopyAssign>, 43);
			V v2(plg::in_place_type<CopyAssign>, 42);
			CopyAssign::reset();
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 1);
			REQUIRE(plg::get<1>(v1).value == 42);
			REQUIRE(CopyAssign::copy_construct == 0);
			REQUIRE(CopyAssign::move_construct == 0);
			REQUIRE(CopyAssign::copy_assign == 1);
		}
#if TEST_HAS_NO_EXCEPTIONS
		using MET = MakeEmptyT;
		{
			using V = plg::variant<int, MET, std::string>;
			V v1(plg::in_place_type<MET>);
			MET& mref = plg::get<1>(v1);
			V v2(plg::in_place_type<MET>);
			try {
				v1 = v2;
				REQUIRE(false);
			} catch (...) {
			}
			REQUIRE(v1.index() == 1);
			REQUIRE(&plg::get<1>(v1) == &mref);
		}
#endif
		// Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
		{
			struct {
				constexpr Result<int> operator()() const {
					using V = plg::variant<int>;
					V v(43);
					V v2(42);
					v = v2;
					return {v.index(), plg::get<0>(v)};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 0);
			static_assert(result.value == 42);
		}
		{
			struct {
				constexpr Result<long> operator()() const {
					using V = plg::variant<int, long, unsigned>;
					V v(43l);
					V v2(42l);
					v = v2;
					return {v.index(), plg::get<1>(v)};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value == 42l);
		}
		{
			struct {
				constexpr Result<int> operator()() const {
					using V = plg::variant<int, TCopyAssign, unsigned>;
					V v(plg::in_place_type<TCopyAssign>, 43);
					V v2(plg::in_place_type<TCopyAssign>, 42);
					v = v2;
					return {v.index(), plg::get<1>(v).value};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value == 42);
		}
		{
			struct {
				constexpr Result<int> operator()() const {
					using V = plg::variant<int, TCopyAssignNTMoveAssign, unsigned>;
					V v(plg::in_place_type<TCopyAssignNTMoveAssign>, 43);
					V v2(plg::in_place_type<TCopyAssignNTMoveAssign>, 42);
					v = v2;
					return {v.index(), plg::get<1>(v).value};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value == 42);
		}
	}
	SECTION("variant& operator=(variant const&) > different_index") {
		{
			using V = plg::variant<int, long, unsigned>;
			V v1(43);
			V v2(42l);
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 1);
			REQUIRE(plg::get<1>(v1) == 42);
		}
		{
			using V = plg::variant<int, CopyAssign, unsigned>;
			CopyAssign::reset();
			V v1(plg::in_place_type<unsigned>, 43u);
			V v2(plg::in_place_type<CopyAssign>, 42);
			REQUIRE(CopyAssign::copy_construct == 0);
			REQUIRE(CopyAssign::move_construct == 0);
			REQUIRE(CopyAssign::alive == 1);
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 1);
			REQUIRE(plg::get<1>(v1).value == 42);
			REQUIRE(CopyAssign::alive == 2);
			REQUIRE(CopyAssign::copy_construct == 1);
			REQUIRE(CopyAssign::move_construct == 1);
			REQUIRE(CopyAssign::copy_assign == 0);
		}
#if TEST_HAS_NO_EXCEPTIONS
		{
			using V = plg::variant<int, CopyThrows, std::string>;
			V v1(plg::in_place_type<std::string>, "hello");
			V v2(plg::in_place_type<CopyThrows>);
			try {
				v1 = v2;
				REQUIRE(false);
			} catch (...) { /* ... */
			}
			// Test that copy construction is used directly if move construction may throw,
			// resulting in a valueless variant if copy throws.
			REQUIRE(v1.valueless_by_exception());
		}
		{
			using V = plg::variant<int, MoveThrows, std::string>;
			V v1(plg::in_place_type<std::string>, "hello");
			V v2(plg::in_place_type<MoveThrows>);
			REQUIRE(MoveThrows::alive == 1);
			// Test that copy construction is used directly if move construction may throw.
			v1 = v2;
			REQUIRE(v1.index() == 1);
			REQUIRE(v2.index() == 1);
			REQUIRE(MoveThrows::alive == 2);
		}
		{
			// Test that direct copy construction is preferred when it cannot throw.
			using V = plg::variant<int, CopyCannotThrow, std::string>;
			V v1(plg::in_place_type<std::string>, "hello");
			V v2(plg::in_place_type<CopyCannotThrow>);
			REQUIRE(CopyCannotThrow::alive == 1);
			v1 = v2;
			REQUIRE(v1.index() == 1);
			REQUIRE(v2.index() == 1);
			REQUIRE(CopyCannotThrow::alive == 2);
		}
		{
			using V = plg::variant<int, CopyThrows, std::string>;
			V v1(plg::in_place_type<CopyThrows>);
			V v2(plg::in_place_type<std::string>, "hello");
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 2);
			REQUIRE(plg::get<2>(v1) == "hello");
			REQUIRE(v2.index() == 2);
			REQUIRE(plg::get<2>(v2) == "hello");
		}
		{
			using V = plg::variant<int, MoveThrows, std::string>;
			V v1(plg::in_place_type<MoveThrows>);
			V v2(plg::in_place_type<std::string>, "hello");
			V& vref = (v1 = v2);
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 2);
			REQUIRE(plg::get<2>(v1) == "hello");
			REQUIRE(v2.index() == 2);
			REQUIRE(plg::get<2>(v2) == "hello");
		}
#endif
		// Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
		{
			struct {
				constexpr Result<long> operator()() const {
					using V = plg::variant<int, long, unsigned>;
					V v(43);
					V v2(42l);
					v = v2;
					return {v.index(), plg::get<1>(v)};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value == 42l);
		}
		{
			struct {
				constexpr Result<int> operator()() const {
					using V = plg::variant<int, TCopyAssign, unsigned>;
					V v(plg::in_place_type<unsigned>, 43u);
					V v2(plg::in_place_type<TCopyAssign>, 42);
					v = v2;
					return {v.index(), plg::get<1>(v).value};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value == 42);
		}
	}
	SECTION("variant& operator=(variant const&) > sfinae") {
		{
			using V = plg::variant<int, long>;
			static_assert(std::is_copy_assignable<V>::value);
		}
		{
			using V = plg::variant<int, CopyOnly>;
			static_assert(std::is_copy_assignable<V>::value);
		}
		{
			using V = plg::variant<int, NoCopy>;
			static_assert(!std::is_copy_assignable<V>::value);
		}
		{
			using V = plg::variant<int, MoveOnly>;
			static_assert(!std::is_copy_assignable<V>::value);
		}
		{
			using V = plg::variant<int, MoveOnlyNT>;
			static_assert(!std::is_copy_assignable<V>::value);
		}
		// Make sure we properly propagate triviality (see P0602R4).
		{
			using V = plg::variant<int, long>;
			static_assert(std::is_trivially_copy_assignable<V>::value);
		}
		{
			using V = plg::variant<int, NTCopyAssign>;
			static_assert(!std::is_trivially_copy_assignable<V>::value);
			static_assert(std::is_copy_assignable<V>::value);
		}
		{
			using V = plg::variant<int, TCopyAssign>;
			static_assert(std::is_trivially_copy_assignable<V>::value);
		}
		{
			using V = plg::variant<int, TCopyAssignNTMoveAssign>;
			static_assert(std::is_trivially_copy_assignable<V>::value);
		}
		{
			using V = plg::variant<int, CopyOnly>;
			static_assert(std::is_trivially_copy_assignable<V>::value);
		}
	}
	SECTION("variant& operator=(variant const&) > not_noexcept") {
		{
			using V = plg::variant<CopyMaybeThrows>;
			static_assert(!std::is_nothrow_copy_assignable<V>::value);
		}
		{
			using V = plg::variant<int, CopyDoesThrow>;
			static_assert(!std::is_nothrow_copy_assignable<V>::value);
		}
	}
	SECTION("variant& operator=(variant const&) > constexpr_copy") {
		// Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
		using V = plg::variant<long, void*, int>;
		static_assert(std::is_trivially_copyable<V>::value);
		static_assert(std::is_trivially_copy_assignable<V>::value);
		static_assert(test_constexpr_assign_imp<0>(V(42l), 101l));
		static_assert(test_constexpr_assign_imp<0>(V(nullptr), 101l));
		static_assert(test_constexpr_assign_imp<1>(V(42l), nullptr));
		static_assert(test_constexpr_assign_imp<2>(V(42l), 101));
	}
}
