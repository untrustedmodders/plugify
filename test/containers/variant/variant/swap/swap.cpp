#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

namespace {
	struct NotSwappable {};
	void swap(NotSwappable&, NotSwappable&) = delete;

	struct NotCopyable {
		NotCopyable() = default;
		NotCopyable(const NotCopyable&) = delete;
		NotCopyable& operator=(const NotCopyable&) = delete;
	};

	struct NotCopyableWithSwap {
		NotCopyableWithSwap() = default;
		NotCopyableWithSwap(const NotCopyableWithSwap&) = delete;
		NotCopyableWithSwap& operator=(const NotCopyableWithSwap&) = delete;
	};
	//void swap(NotCopyableWithSwap&, NotCopyableWithSwap) {}

	struct NotMoveAssignable {
		NotMoveAssignable() = default;
		NotMoveAssignable(NotMoveAssignable&&) = default;
		NotMoveAssignable& operator=(NotMoveAssignable&&) = delete;
	};

	struct NotMoveAssignableWithSwap {
		NotMoveAssignableWithSwap() = default;
		NotMoveAssignableWithSwap(NotMoveAssignableWithSwap&&) = default;
		NotMoveAssignableWithSwap& operator=(NotMoveAssignableWithSwap&&) = delete;
	};
	void swap(NotMoveAssignableWithSwap&, NotMoveAssignableWithSwap&) noexcept {}

	template<bool Throws>
	void do_throw() {}

	template<>
	void do_throw<true>() {
#if TEST_HAS_NO_EXCEPTIONS
		throw 42;
#endif
	}

	template<bool NT_Copy, bool NT_Move, bool NT_CopyAssign, bool NT_MoveAssign,
			 bool NT_Swap, bool EnableSwap = true>
	struct NothrowTypeImp {
		static int move_called;
		static int move_assign_called;
		static int swap_called;
		static void reset() { move_called = move_assign_called = swap_called = 0; }
		NothrowTypeImp() = default;
		explicit NothrowTypeImp(int v) : value(v) {}
		NothrowTypeImp(const NothrowTypeImp& o) noexcept(NT_Copy) : value(o.value) {
			REQUIRE(false);
		}// never called by test
		NothrowTypeImp(NothrowTypeImp&& o) noexcept(NT_Move) : value(o.value) {
			++move_called;
			do_throw<!NT_Move>();
			o.value = -1;
		}
		NothrowTypeImp& operator=(const NothrowTypeImp&) noexcept(NT_CopyAssign) {
			REQUIRE(false);
			return *this;
		}// never called by the tests
		NothrowTypeImp& operator=(NothrowTypeImp&& o) noexcept(NT_MoveAssign) {
			++move_assign_called;
			do_throw<!NT_MoveAssign>();
			value = o.value;
			o.value = -1;
			return *this;
		}
		int value;
	};
	template<bool NT_Copy, bool NT_Move, bool NT_CopyAssign, bool NT_MoveAssign,
			 bool NT_Swap, bool EnableSwap>
	int NothrowTypeImp<NT_Copy, NT_Move, NT_CopyAssign, NT_MoveAssign, NT_Swap,
					   EnableSwap>::move_called = 0;
	template<bool NT_Copy, bool NT_Move, bool NT_CopyAssign, bool NT_MoveAssign,
			 bool NT_Swap, bool EnableSwap>
	int NothrowTypeImp<NT_Copy, NT_Move, NT_CopyAssign, NT_MoveAssign, NT_Swap,
					   EnableSwap>::move_assign_called = 0;
	template<bool NT_Copy, bool NT_Move, bool NT_CopyAssign, bool NT_MoveAssign,
			 bool NT_Swap, bool EnableSwap>
	int NothrowTypeImp<NT_Copy, NT_Move, NT_CopyAssign, NT_MoveAssign, NT_Swap,
					   EnableSwap>::swap_called = 0;

	template<bool NT_Copy, bool NT_Move, bool NT_CopyAssign, bool NT_MoveAssign,
			 bool NT_Swap>
	void swap(NothrowTypeImp<NT_Copy, NT_Move, NT_CopyAssign, NT_MoveAssign,
							 NT_Swap, true>& lhs,
			  NothrowTypeImp<NT_Copy, NT_Move, NT_CopyAssign, NT_MoveAssign,
							 NT_Swap, true>& rhs) noexcept(NT_Swap) {
		lhs.swap_called++;
		do_throw<!NT_Swap>();
		int tmp = lhs.value;
		lhs.value = rhs.value;
		rhs.value = tmp;
	}

	// throwing copy, nothrow move ctor/assign, no swap provided
	using NothrowMoveable = NothrowTypeImp<false, true, false, true, false, false>;
	// throwing copy and move assign, nothrow move ctor, no swap provided
	using NothrowMoveCtor = NothrowTypeImp<false, true, false, false, false, false>;
	// nothrow move ctor, throwing move assignment, swap provided
	using NothrowMoveCtorWithThrowingSwap =
			NothrowTypeImp<false, true, false, false, false, true>;
	// throwing move ctor, nothrow move assignment, no swap provided
	using ThrowingMoveCtor =
			NothrowTypeImp<false, false, false, true, false, false>;
	// throwing special members, nothrowing swap
	using ThrowingTypeWithNothrowSwap =
			NothrowTypeImp<false, false, false, false, true, true>;
	using NothrowTypeWithThrowingSwap =
			NothrowTypeImp<true, true, true, true, false, true>;
	// throwing move assign with nothrow move and nothrow swap
	using ThrowingMoveAssignNothrowMoveCtorWithSwap =
			NothrowTypeImp<false, true, false, false, true, true>;
	// throwing move assign with nothrow move but no swap.
	using ThrowingMoveAssignNothrowMoveCtor =
			NothrowTypeImp<false, true, false, false, false, false>;

	struct NonThrowingNonNoexceptType {
		static int move_called;
		static void reset() { move_called = 0; }
		NonThrowingNonNoexceptType() = default;
		NonThrowingNonNoexceptType(int v) : value(v) {}
		NonThrowingNonNoexceptType(NonThrowingNonNoexceptType&& o) noexcept(false)
			: value(o.value) {
			++move_called;
			o.value = -1;
		}
		NonThrowingNonNoexceptType&
		operator=(NonThrowingNonNoexceptType&&) noexcept(false) {
			REQUIRE(false);// never called by the tests.
			return *this;
		}
		int value;
	};
	int NonThrowingNonNoexceptType::move_called = 0;

	struct ThrowsOnSecondMove {
		int value;
		int move_count;
		ThrowsOnSecondMove(int v) : value(v), move_count(0) {}
		ThrowsOnSecondMove(ThrowsOnSecondMove&& o) noexcept(false)
			: value(o.value), move_count(o.move_count + 1) {
			if (move_count == 2)
				do_throw<true>();
			o.value = -1;
		}
		ThrowsOnSecondMove& operator=(ThrowsOnSecondMove&&) {
			REQUIRE(false);// not called by test
			return *this;
		}
	};

	template<class Var>
	constexpr auto has_swap_member_imp(int)
			-> decltype(std::declval<Var&>().swap(std::declval<Var&>()), true) {
		return true;
	}

	template<class Var>
	constexpr auto has_swap_member_imp(long) -> bool {
		return false;
	}

	template<class Var>
	constexpr bool has_swap_member() {
		return has_swap_member_imp<Var>(0);
	}

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
}// namespace

TEST_CASE("variant > swap", "[variant]") {
#if TEST_HAS_NO_EXCEPTIONS
	SECTION("void swap(variant& rhs) noexcept > valueless_by_exception") {
		using V = plg::variant<int, MakeEmptyT>;
		{// both empty
			V v1;
			makeEmpty(v1);
			V v2;
			makeEmpty(v2);
			REQUIRE(MakeEmptyT::alive == 0);
			{// member swap
				v1.swap(v2);
				REQUIRE(v1.valueless_by_exception());
				REQUIRE(v2.valueless_by_exception());
				REQUIRE(MakeEmptyT::alive == 0);
			}
			{// non-member swap
				swap(v1, v2);
				REQUIRE(v1.valueless_by_exception());
				REQUIRE(v2.valueless_by_exception());
				REQUIRE(MakeEmptyT::alive == 0);
			}
		}
		{// only one empty
			V v1(42);
			V v2;
			makeEmpty(v2);
			{// member swap
				v1.swap(v2);
				REQUIRE(v1.valueless_by_exception());
				REQUIRE(plg::get<0>(v2) == 42);
				// swap again
				v2.swap(v1);
				REQUIRE(v2.valueless_by_exception());
				REQUIRE(plg::get<0>(v1) == 42);
			}
			{// non-member swap
				swap(v1, v2);
				REQUIRE(v1.valueless_by_exception());
				REQUIRE(plg::get<0>(v2) == 42);
				// swap again
				swap(v1, v2);
				REQUIRE(v2.valueless_by_exception());
				REQUIRE(plg::get<0>(v1) == 42);
			}
		}
	}
#endif
	SECTION("void swap(variant& rhs) noexcept > same_alternative") {
		{
			using T = ThrowingTypeWithNothrowSwap;
			using V = plg::variant<T, int>;
			T::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<0>, 100);
			v1.swap(v2);
			REQUIRE(T::swap_called == 1);
			REQUIRE(plg::get<0>(v1).value == 100);
			REQUIRE(plg::get<0>(v2).value == 42);
			swap(v1, v2);
			REQUIRE(T::swap_called == 2);
			REQUIRE(plg::get<0>(v1).value == 42);
			REQUIRE(plg::get<0>(v2).value == 100);
		}
		{
			using T = NothrowMoveable;
			using V = plg::variant<T, int>;
			T::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<0>, 100);
			v1.swap(v2);
			REQUIRE(T::swap_called == 0);
			REQUIRE(T::move_called == 1);
			REQUIRE(T::move_assign_called == 2);
			REQUIRE(plg::get<0>(v1).value == 100);
			REQUIRE(plg::get<0>(v2).value == 42);
			T::reset();
			swap(v1, v2);
			REQUIRE(T::swap_called == 0);
			REQUIRE(T::move_called == 1);
			REQUIRE(T::move_assign_called == 2);
			REQUIRE(plg::get<0>(v1).value == 42);
			REQUIRE(plg::get<0>(v2).value == 100);
		}
#if TEST_HAS_NO_EXCEPTIONS
		{
			using T = NothrowTypeWithThrowingSwap;
			using V = plg::variant<T, int>;
			T::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<0>, 100);
			try {
				v1.swap(v2);
				REQUIRE(false);
			} catch (int) {
			}
			REQUIRE(T::swap_called == 1);
			REQUIRE(T::move_called == 0);
			REQUIRE(T::move_assign_called == 0);
			REQUIRE(plg::get<0>(v1).value == 42);
			REQUIRE(plg::get<0>(v2).value == 100);
		}
		{
			using T = ThrowingMoveCtor;
			using V = plg::variant<T, int>;
			T::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<0>, 100);
			try {
				v1.swap(v2);
				REQUIRE(false);
			} catch (int) {
			}
			REQUIRE(T::move_called == 1);// call threw
			REQUIRE(T::move_assign_called == 0);
			REQUIRE(plg::get<0>(v1).value ==
					42);// throw happened before v1 was moved from
			REQUIRE(plg::get<0>(v2).value == 100);
		}
		{
			using T = ThrowingMoveAssignNothrowMoveCtor;
			using V = plg::variant<T, int>;
			T::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<0>, 100);
			try {
				v1.swap(v2);
				REQUIRE(false);
			} catch (int) {
			}
			REQUIRE(T::move_called == 1);
			REQUIRE(T::move_assign_called == 1); // call threw and didn't complete
			REQUIRE(plg::get<0>(v1).value == -1);// v1 was moved from
			REQUIRE(plg::get<0>(v2).value == 100);
		}
#endif
	}
#if TEST_HAS_NO_EXCEPTIONS
	SECTION("void swap(variant& rhs) noexcept > different_alternatives") {
		{
			using T = NothrowMoveCtorWithThrowingSwap;
			using V = plg::variant<T, int>;
			T::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<1>, 100);
			v1.swap(v2);
			REQUIRE(T::swap_called == 0);
			// The libc++ implementation double copies the argument, and not
			// the variant swap is called on.
			REQUIRE(T::move_called == 1);
			REQUIRE(T::move_called <= 2);
			REQUIRE(T::move_assign_called == 0);
			REQUIRE(plg::get<1>(v1) == 100);
			REQUIRE(plg::get<0>(v2).value == 42);
			T::reset();
			swap(v1, v2);
			REQUIRE(T::swap_called == 0);
			REQUIRE(T::move_called == 2);
			REQUIRE(T::move_called <= 2);
			REQUIRE(T::move_assign_called == 0);
			REQUIRE(plg::get<0>(v1).value == 42);
			REQUIRE(plg::get<1>(v2) == 100);
		}
		{
			using T1 = ThrowingTypeWithNothrowSwap;
			using T2 = NonThrowingNonNoexceptType;
			using V = plg::variant<T1, T2>;
			T1::reset();
			T2::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<1>, 100);
			try {
				v1.swap(v2);
				REQUIRE(false);
			} catch (int) {
			}
			REQUIRE(T1::swap_called == 0);
			REQUIRE(T1::move_called == 1);// throws
			REQUIRE(T1::move_assign_called == 0);
			// FIXME: libc++ shouldn't move from T2 here.
			REQUIRE(T2::move_called == 1);
			REQUIRE(T2::move_called <= 1);
			REQUIRE(plg::get<0>(v1).value == 42);
			if (T2::move_called != 0)
				REQUIRE(v2.valueless_by_exception());
			else
				REQUIRE(plg::get<1>(v2).value == 100);
		}
		{
			using T1 = NonThrowingNonNoexceptType;
			using T2 = ThrowingTypeWithNothrowSwap;
			using V = plg::variant<T1, T2>;
			T1::reset();
			T2::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<1>, 100);
			try {
				v1.swap(v2);
				REQUIRE(false);
			} catch (int) {
			}
			REQUIRE(T1::move_called == 0);
			REQUIRE(T1::move_called <= 1);
			REQUIRE(T2::swap_called == 0);
			REQUIRE(T2::move_called == 1);// throws
			REQUIRE(T2::move_assign_called == 0);
			if (T1::move_called != 0)
				REQUIRE(v1.valueless_by_exception());
			else
				REQUIRE(plg::get<0>(v1).value == 42);
			REQUIRE(plg::get<1>(v2).value == 100);
		}
		{
			using T1 = ThrowsOnSecondMove;
			using T2 = NonThrowingNonNoexceptType;
			using V = plg::variant<T1, T2>;
			T2::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<1>, 100);
			v1.swap(v2);
			REQUIRE(T2::move_called == 2);
			REQUIRE(plg::get<1>(v1).value == 100);
			REQUIRE(plg::get<0>(v2).value == 42);
			REQUIRE(plg::get<0>(v2).move_count == 1);
		}
		{
			using T1 = NonThrowingNonNoexceptType;
			using T2 = ThrowsOnSecondMove;
			using V = plg::variant<T1, T2>;
			T1::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<1>, 100);
			try {
				v1.swap(v2);
				REQUIRE(false);
			} catch (int) {
			}
			REQUIRE(T1::move_called == 1);
			REQUIRE(v1.valueless_by_exception());
			REQUIRE(plg::get<0>(v2).value == 42);
		}
		{

			using T1 = ThrowingTypeWithNothrowSwap;
			using T2 = NothrowMoveable;
			using V = plg::variant<T1, T2>;
			T1::reset();
			T2::reset();
			V v1(plg::in_place_index<0>, 42);
			V v2(plg::in_place_index<1>, 100);
			try {
				v1.swap(v2);
				REQUIRE(false);
			} catch (int) {
			}
			REQUIRE(T1::swap_called == 0);
			REQUIRE(T1::move_called == 1);
			REQUIRE(T1::move_assign_called == 0);
			REQUIRE(T2::swap_called == 0);
			REQUIRE(T2::move_called == 2);
			REQUIRE(T2::move_assign_called == 0);
			REQUIRE(plg::get<0>(v1).value == 42);
			REQUIRE(plg::get<1>(v2).value == 100);
			// swap again, but call v2's swap.
			T1::reset();
			T2::reset();
			try {
				v2.swap(v1);
				REQUIRE(false);
			} catch (int) {
			}
			REQUIRE(T1::swap_called == 0);
			REQUIRE(T1::move_called == 1);
			REQUIRE(T1::move_assign_called == 0);
			REQUIRE(T2::swap_called == 0);
			REQUIRE(T2::move_called == 2);
			REQUIRE(T2::move_assign_called == 0);
			REQUIRE(plg::get<0>(v1).value == 42);
			REQUIRE(plg::get<1>(v2).value == 100);
		}
	}
#endif
	SECTION("void swap(variant& rhs) noexcept > sfinae") {
		{
			// This variant type does not provide either a member or non-member swap
			// but is still swappable via the generic swap algorithm, since the
			// variant is move constructible and move assignable.
			using V = plg::variant<int, NotSwappable>;
			static_assert(!has_swap_member<V>());
			static_assert(std::is_swappable_v<V>);
		}
		{
			using V = plg::variant<int, NotCopyable>;
			static_assert(!has_swap_member<V>());
			static_assert(!std::is_swappable_v<V>);
		}
		{
			using V = plg::variant<int, NotCopyableWithSwap>;
			static_assert(!has_swap_member<V>());
			static_assert(!std::is_swappable_v<V>);
		}
		{
			using V = plg::variant<int, NotMoveAssignable>;
			static_assert(!has_swap_member<V>());
			static_assert(!std::is_swappable_v<V>);
		}
	}
	SECTION("void swap(variant& rhs) noexcept > noexcept") {
		{
			using V = plg::variant<int, NothrowMoveable>;
			static_assert(std::is_swappable_v<V> && has_swap_member<V>());
			static_assert(std::is_nothrow_swappable_v<V>);
			// instantiate swap
			V v1, v2;
			v1.swap(v2);
			swap(v1, v2);
		}
		{
			using V = plg::variant<int, NothrowMoveCtor>;
			static_assert(std::is_swappable_v<V> && has_swap_member<V>());
			static_assert(!std::is_nothrow_swappable_v<V>);
			// instantiate swap
			V v1, v2;
			v1.swap(v2);
			swap(v1, v2);
		}
		{
			using V = plg::variant<int, ThrowingTypeWithNothrowSwap>;
			static_assert(std::is_swappable_v<V> && has_swap_member<V>());
			static_assert(!std::is_nothrow_swappable_v<V>);
			// instantiate swap
			V v1, v2;
			v1.swap(v2);
			swap(v1, v2);
		}
		{
			using V = plg::variant<int, ThrowingMoveAssignNothrowMoveCtor>;
			static_assert(std::is_swappable_v<V> && has_swap_member<V>());
			static_assert(!std::is_nothrow_swappable_v<V>);
			// instantiate swap
			V v1, v2;
			v1.swap(v2);
			swap(v1, v2);
		}
		{
			using V = plg::variant<int, ThrowingMoveAssignNothrowMoveCtorWithSwap>;
			static_assert(std::is_swappable_v<V> && has_swap_member<V>());
			// plg-FIXME : this is a libc++ extension? silencing it for now
			//static_assert(std::is_nothrow_swappable_v<V>);
			// instantiate swap
			V v1, v2;
			v1.swap(v2);
			swap(v1, v2);
		}
		{
			using V = plg::variant<int, NotMoveAssignableWithSwap>;
			static_assert(std::is_swappable_v<V> && has_swap_member<V>());
			// plg-FIXME : this is a libc++ extension? silencing it for now
			//static_assert(std::is_nothrow_swappable_v<V>);
			// instantiate swap
			V v1, v2;
			v1.swap(v2);
			swap(v1, v2);
		}
		{
			// This variant type does not provide either a member or non-member swap
			// but is still swappable via the generic swap algorithm, since the
			// variant is move constructible and move assignable.
			using V = plg::variant<int, NotSwappable>;
			static_assert(!has_swap_member<V>());
			static_assert(std::is_swappable_v<V>);
			static_assert(std::is_nothrow_swappable_v<V>);
			V v1, v2;
			// plg : the original LLVM file does not has using std::swap, a mistake?
			using std::swap;
			swap(v1, v2);
		}
	}
}

// This is why variant should SFINAE member swap. :-)
template class plg::variant<int, NotSwappable>;
