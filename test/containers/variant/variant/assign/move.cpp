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
		MoveOnly& operator=(const MoveOnly&) = delete;
		MoveOnly& operator=(MoveOnly&&) = default;
	};

	struct MoveOnlyNT {
		MoveOnlyNT(const MoveOnlyNT&) = delete;
		MoveOnlyNT(MoveOnlyNT&&) {}
		MoveOnlyNT& operator=(const MoveOnlyNT&) = delete;
		MoveOnlyNT& operator=(MoveOnlyNT&&) = default;
	};

	struct MoveOnlyOddNothrow {
		MoveOnlyOddNothrow(MoveOnlyOddNothrow&&) noexcept(false) {}
		MoveOnlyOddNothrow(const MoveOnlyOddNothrow&) = delete;
		MoveOnlyOddNothrow& operator=(MoveOnlyOddNothrow&&) noexcept = default;
		MoveOnlyOddNothrow& operator=(const MoveOnlyOddNothrow&) = delete;
	};

	struct MoveAssignOnly {
		MoveAssignOnly(MoveAssignOnly&&) = delete;
		MoveAssignOnly& operator=(MoveAssignOnly&&) = default;
	};

	struct MoveAssign {
		static int move_construct;
		static int move_assign;
		static void reset() { move_construct = move_assign = 0; }
		MoveAssign(int v) : value(v) {}
		MoveAssign(MoveAssign&& o) : value(o.value) {
			++move_construct;
			o.value = -1;
		}
		MoveAssign& operator=(MoveAssign&& o) {
			value = o.value;
			++move_assign;
			o.value = -1;
			return *this;
		}
		int value;
	};

	int MoveAssign::move_construct = 0;
	int MoveAssign::move_assign = 0;

	struct NTMoveAssign {
		constexpr NTMoveAssign(int v) : value(v) {}
		NTMoveAssign(const NTMoveAssign&) = default;
		NTMoveAssign(NTMoveAssign&&) = default;
		NTMoveAssign& operator=(const NTMoveAssign& that) = default;
		NTMoveAssign& operator=(NTMoveAssign&& that) {
			value = that.value;
			that.value = -1;
			return *this;
		};
		int value;
	};

	static_assert(!std::is_trivially_move_assignable<NTMoveAssign>::value);
	static_assert(std::is_move_assignable<NTMoveAssign>::value);

	struct TMoveAssign {
		constexpr TMoveAssign(int v) : value(v) {}
		TMoveAssign(const TMoveAssign&) = delete;
		TMoveAssign(TMoveAssign&&) = default;
		TMoveAssign& operator=(const TMoveAssign&) = delete;
		TMoveAssign& operator=(TMoveAssign&&) = default;
		int value;
	};

	static_assert(std::is_trivially_move_assignable<TMoveAssign>::value);

	struct TMoveAssignNTCopyAssign {
		constexpr TMoveAssignNTCopyAssign(int v) : value(v) {}
		TMoveAssignNTCopyAssign(const TMoveAssignNTCopyAssign&) = default;
		TMoveAssignNTCopyAssign(TMoveAssignNTCopyAssign&&) = default;
		TMoveAssignNTCopyAssign& operator=(const TMoveAssignNTCopyAssign& that) {
			value = that.value;
			return *this;
		}
		TMoveAssignNTCopyAssign& operator=(TMoveAssignNTCopyAssign&&) = default;
		int value;
	};

	static_assert(std::is_trivially_move_assignable_v<TMoveAssignNTCopyAssign>);

	struct TrivialCopyNontrivialMove {
		TrivialCopyNontrivialMove(TrivialCopyNontrivialMove const&) = default;
		TrivialCopyNontrivialMove(TrivialCopyNontrivialMove&&) noexcept {}
		TrivialCopyNontrivialMove& operator=(TrivialCopyNontrivialMove const&) = default;
		TrivialCopyNontrivialMove& operator=(TrivialCopyNontrivialMove&&) noexcept {
			return *this;
		}
	};

	static_assert(std::is_trivially_copy_assignable_v<TrivialCopyNontrivialMove>);
	static_assert(!std::is_trivially_move_assignable_v<TrivialCopyNontrivialMove>);

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

	template<size_t NewIdx, class ValueType>
	constexpr bool test_constexpr_assign_imp(
			plg::variant<long, void*, int>&& v, ValueType&& new_value) {
		plg::variant<long, void*, int> v2(
				std::forward<ValueType>(new_value));
		const auto cp = v2;
		v = std::move(v2);
		return v.index() == NewIdx &&
			   plg::get<NewIdx>(v) == plg::get<NewIdx>(cp);
	}
} // namespace

TEST_CASE("variant operator > assignment > move", "[variant]") {
#if TEST_HAS_NO_EXCEPTIONS
	SECTION("variant& operator=(variant&&) noexcept(see below) > empty_empty") {
		using MET = MakeEmptyT;
		{
			using V = plg::variant<int, long, MET>;
			V v1(plg::in_place_index<0>);
			makeEmpty(v1);
			V v2(plg::in_place_index<0>);
			makeEmpty(v2);
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.valueless_by_exception());
			// REQUIRE(v1.index() == plg::variant<int, float>::npos);
		}
	}
	SECTION("variant& operator=(variant&&) noexcept(see below) > non_empty_empty") {
		using MET = MakeEmptyT;
		{
			using V = plg::variant<int, MET>;
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<0>);
			makeEmpty(v2);
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.valueless_by_exception());
			// REQUIRE(v1.index() == plg::variant<int, float>::npos);
		}
		{
			using V = plg::variant<int, MET, std::string>;
			V v1(plg::in_place_index<2>, "hello");
			V v2(plg::in_place_index<0>);
			makeEmpty(v2);
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.valueless_by_exception());
			// REQUIRE(v1.index() == plg::variant<int, float>::npos);
		}
	}
	SECTION("variant& operator=(variant&&) noexcept(see below) > empty_non_empty") {
		using MET = MakeEmptyT;
		{
			using V = plg::variant<int, MET>;
			V v1(plg::in_place_index<0>);
			makeEmpty(v1);
			V v2(plg::in_place_index<0>, 42);
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 0);
			REQUIRE(plg::get<0>(v1) == 42);
		}
		{
			using V = plg::variant<int, MET, std::string>;
			V v1(plg::in_place_index<0>);
			makeEmpty(v1);
			V v2(plg::in_place_type<std::string>, "hello");
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 2);
			REQUIRE(plg::get<2>(v1) == "hello");
		}
	}
#endif
	SECTION("variant& operator=(variant&&) noexcept(see below) > same_index") {
		{
			using V = plg::variant<int>;
			V v1(43);
			V v2(42);
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 0);
			REQUIRE(plg::get<0>(v1) == 42);
		}
		{
			using V = plg::variant<int, long, unsigned>;
			V v1(43l);
			V v2(42l);
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 1);
			REQUIRE(plg::get<1>(v1) == 42);
		}
		{
			using V = plg::variant<int, MoveAssign, unsigned>;
			V v1(plg::in_place_type<MoveAssign>, 43);
			V v2(plg::in_place_type<MoveAssign>, 42);
			MoveAssign::reset();
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 1);
			REQUIRE(plg::get<1>(v1).value == 42);
			REQUIRE(MoveAssign::move_construct == 0);
			REQUIRE(MoveAssign::move_assign == 1);
		}
#if TEST_HAS_NO_EXCEPTIONS
		using MET = MakeEmptyT;
		{
			using V = plg::variant<int, MET, std::string>;
			V v1(plg::in_place_type<MET>);
			MET& mref = plg::get<1>(v1);
			V v2(plg::in_place_type<MET>);
			try {
				v1 = std::move(v2);
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
					v = std::move(v2);
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
					v = std::move(v2);
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
					using V = plg::variant<int, TMoveAssign, unsigned>;
					V v(plg::in_place_type<TMoveAssign>, 43);
					V v2(plg::in_place_type<TMoveAssign>, 42);
					v = std::move(v2);
					return {v.index(), plg::get<1>(v).value};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value == 42);
		}
	}
	SECTION("variant& operator=(variant&&) noexcept(see below) > different_index") {
		{
			using V = plg::variant<int, long, unsigned>;
			V v1(43);
			V v2(42l);
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 1);
			REQUIRE(plg::get<1>(v1) == 42);
		}
		{
			using V = plg::variant<int, MoveAssign, unsigned>;
			V v1(plg::in_place_type<unsigned>, 43u);
			V v2(plg::in_place_type<MoveAssign>, 42);
			MoveAssign::reset();
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 1);
			REQUIRE(plg::get<1>(v1).value == 42);
			REQUIRE(MoveAssign::move_construct == 1);
			REQUIRE(MoveAssign::move_assign == 0);
		}
#if TEST_HAS_NO_EXCEPTIONS
		using MET = MakeEmptyT;
		{
			using V = plg::variant<int, MET, std::string>;
			V v1(plg::in_place_type<int>);
			V v2(plg::in_place_type<MET>);
			try {
				v1 = std::move(v2);
				REQUIRE(false);
			} catch (...) {
			}
			REQUIRE(v1.valueless_by_exception());
			// REQUIRE(v1.index() == plg::variant<int, float>::npos);
		}
		{
			using V = plg::variant<int, MET, std::string>;
			V v1(plg::in_place_type<MET>);
			V v2(plg::in_place_type<std::string>, "hello");
			V& vref = (v1 = std::move(v2));
			REQUIRE(&vref == &v1);
			REQUIRE(v1.index() == 2);
			REQUIRE(plg::get<2>(v1) == "hello");
		}
#endif
		// Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
		{
			struct {
				constexpr Result<long> operator()() const {
					using V = plg::variant<int, long, unsigned>;
					V v(43);
					V v2(42l);
					v = std::move(v2);
					return {v.index(), plg::get<1>(v)};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value == 42l);
		}
		{
			struct {
				constexpr Result<long> operator()() const {
					using V = plg::variant<int, TMoveAssign, unsigned>;
					V v(plg::in_place_type<unsigned>, 43u);
					V v2(plg::in_place_type<TMoveAssign>, 42);
					v = std::move(v2);
					return {v.index(), plg::get<1>(v).value};
				}
			} test;
			constexpr auto result = test();
			static_assert(result.index == 1);
			static_assert(result.value == 42);
		}
	}
	SECTION("variant& operator=(variant&&) noexcept(see below) > sfinae") {
			{
				using V = plg::variant<int, long>;
				static_assert(std::is_move_assignable<V>::value);
			}
			{
				using V = plg::variant<int, CopyOnly>;
				static_assert(std::is_move_assignable<V>::value);
			}
			{
				using V = plg::variant<int, NoCopy>;
				static_assert(!std::is_move_assignable<V>::value);
			}
			{
				using V = plg::variant<int, MoveOnly>;
				static_assert(std::is_move_assignable<V>::value);
			}
			{
				using V = plg::variant<int, MoveOnlyNT>;
				static_assert(std::is_move_assignable<V>::value);
			}
			{
				// variant only provides move assignment when the types also provide
				// a move constructor.
				using V = plg::variant<int, MoveAssignOnly>;
				static_assert(!std::is_move_assignable<V>::value);
			}

			// Make sure we properly propagate triviality (see P0602R4).
			{
				using V = plg::variant<int, long>;
				static_assert(std::is_trivially_move_assignable<V>::value);
			}
			{
				using V = plg::variant<int, NTMoveAssign>;
				static_assert(!std::is_trivially_move_assignable<V>::value);
				static_assert(std::is_move_assignable<V>::value);
			}
			{
				using V = plg::variant<int, TMoveAssign>;
				static_assert(std::is_trivially_move_assignable<V>::value);
			}
			{
				using V = plg::variant<int, TMoveAssignNTCopyAssign>;
				static_assert(std::is_trivially_move_assignable<V>::value);
			}
			{
				using V = plg::variant<int, TrivialCopyNontrivialMove>;
				static_assert(!std::is_trivially_move_assignable<V>::value);
			}
			{
				using V = plg::variant<int, CopyOnly>;
				static_assert(std::is_trivially_move_assignable<V>::value);
			}
	}
	SECTION("variant& operator=(variant&&) noexcept(see below) > noexcept") {
		{
			using V = plg::variant<int>;
			static_assert(std::is_nothrow_move_assignable<V>::value);
		}
		{
			using V = plg::variant<MoveOnly>;
			static_assert(std::is_nothrow_move_assignable<V>::value);
		}
		{
			using V = plg::variant<int, long>;
			static_assert(std::is_nothrow_move_assignable<V>::value);
		}
		{
			using V = plg::variant<int, MoveOnly>;
			static_assert(std::is_nothrow_move_assignable<V>::value);
		}
		{
			using V = plg::variant<MoveOnlyNT>;
			static_assert(!std::is_nothrow_move_assignable<V>::value);
		}
		{
			using V = plg::variant<MoveOnlyOddNothrow>;
			static_assert(!std::is_nothrow_move_assignable<V>::value);
		}
	}
	SECTION("variant& operator=(variant&&) noexcept(see below) > move_assignment") {
		// Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
		using V = plg::variant<long, void*, int>;
		static_assert(std::is_trivially_copyable<V>::value);
		static_assert(std::is_trivially_move_assignable<V>::value);
		static_assert(test_constexpr_assign_imp<0>(V(42l), 101l));
		static_assert(test_constexpr_assign_imp<0>(V(nullptr), 101l));
		static_assert(test_constexpr_assign_imp<1>(V(42l), nullptr));
		static_assert(test_constexpr_assign_imp<2>(V(42l), 101));
	}
}
