#include <catch_amalgamated.hpp>

#include <plugifyvariant.hpp>

#include "app/variant_tester.hpp"

namespace MetaHelpers {
	struct Dummy {
		Dummy() = default;
	};

	struct ThrowsCtorT {
		ThrowsCtorT(int) noexcept(false) {}
		ThrowsCtorT& operator=(int) noexcept { return *this; }
	};

	struct ThrowsAssignT {
		ThrowsAssignT(int) noexcept {}
		ThrowsAssignT& operator=(int) noexcept(false) { return *this; }
	};

	struct NoThrowT {
		NoThrowT(int) noexcept {}
		NoThrowT& operator=(int) noexcept { return *this; }
	};

}// namespace MetaHelpers

#if TEST_HAS_NO_EXCEPTIONS
namespace RuntimeHelpers {
	struct ThrowsCtorT {
		int value;
		ThrowsCtorT() : value(0) {}
		ThrowsCtorT(int) noexcept(false) { throw 42; }
		ThrowsCtorT& operator=(int v) noexcept {
			value = v;
			return *this;
		}
	};

	struct MoveCrashes {
		int value;
		MoveCrashes(int v = 0) noexcept : value{v} {}
		MoveCrashes(MoveCrashes&&) noexcept { REQUIRE(false); }
		MoveCrashes& operator=(MoveCrashes&&) noexcept {
			REQUIRE(false);
			return *this;
		}
		MoveCrashes& operator=(int v) noexcept {
			value = v;
			return *this;
		}
	};

	struct ThrowsCtorTandMove {
		int value;
		ThrowsCtorTandMove() : value(0) {}
		ThrowsCtorTandMove(int) noexcept(false) { throw 42; }
		ThrowsCtorTandMove(ThrowsCtorTandMove&&) noexcept(false) { REQUIRE(false); }
		ThrowsCtorTandMove& operator=(int v) noexcept {
			value = v;
			return *this;
		}
	};

	struct ThrowsAssignT {
		int value;
		ThrowsAssignT() : value(0) {}
		ThrowsAssignT(int v) noexcept : value(v) {}
		ThrowsAssignT& operator=(int) noexcept(false) { throw 42; }
	};

	struct NoThrowT {
		int value;
		NoThrowT() : value(0) {}
		NoThrowT(int v) noexcept : value(v) {}
		NoThrowT& operator=(int v) noexcept {
			value = v;
			return *this;
		}
	};

}// namespace RuntimeHelpers
#endif

TEST_CASE("variant operator > assignment > typed move", "[variant]") {
	SECTION("variant& operator=(T&&) noexcept(see below) > basic") {
	{
		plg::variant<int> v(43);
		v = 42;
		REQUIRE(v.index() == 0);
		REQUIRE(plg::get<0>(v) == 42);
	}
	{
		plg::variant<int, long> v(43l);
		v = 42;
		REQUIRE(v.index() == 0);
		REQUIRE(plg::get<0>(v) == 42);
		v = 43l;
		REQUIRE(v.index() == 1);
		REQUIRE(plg::get<1>(v) == 43);
	}
	#ifndef TEST_VARIANT_ALLOWS_NARROWING_CONVERSIONS
	{
		plg::variant<unsigned, long> v;
		v = 42;
		REQUIRE(v.index() == 1);
		REQUIRE(plg::get<1>(v) == 42);
		v = 43u;
		REQUIRE(v.index() == 0);
		REQUIRE(plg::get<0>(v) == 43);
	}
	#endif
	{
		plg::variant<std::string, bool> v = true;
		v = "bar";
		REQUIRE(v.index() == 0);
		REQUIRE(plg::get<0>(v) == "bar");
	}
	{
		plg::variant<bool, std::unique_ptr<int>> v;
		v = nullptr;
		REQUIRE(v.index() == 1);
		REQUIRE(plg::get<1>(v) == nullptr);
	}
	{
		plg::variant<bool, int> v = 42;
		v = false;
		REQUIRE(v.index() == 0);
		REQUIRE(!plg::get<0>(v));
		bool lvt = true;
		v = lvt;
		REQUIRE(v.index() == 0);
		REQUIRE(plg::get<0>(v));
	}
#if !TEST_VARIANT_HAS_NO_REFERENCES
	{
		using V = plg::variant<int&, int&&, long>;
		int x = 42;
		V v(43l);
		v = x;
		REQUIRE(v.index() == 0);
		REQUIRE(&plg::get<0>(v) == &x);
		v = std::move(x);
		REQUIRE(v.index() == 1);
		REQUIRE(&plg::get<1>(v) == &x);
		// 'long' is selected by FUN(const int &) since 'const int &' cannot bind
		// to 'int&'.
		const int& cx = x;
		v = cx;
		REQUIRE(v.index() == 2);
		REQUIRE(plg::get<2>(v) == 42);
	}
#endif// TEST_VARIANT_HAS_NO_REFERENCES
	}

#if TEST_HAS_NO_EXCEPTIONS
	SECTION("variant& operator=(T&&) noexcept(see below) > performs_construction") {
		using namespace RuntimeHelpers;
		{
			using V = plg::variant<std::string, ThrowsCtorT>;
			V v(plg::in_place_type<std::string>, "hello");
			try {
				v = 42;
				REQUIRE(false);
			} catch (...) { /* ... */
			}
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v) == "hello");
		}
		{
			using V = plg::variant<ThrowsAssignT, std::string>;
			V v(plg::in_place_type<std::string>, "hello");
			v = 42;
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v).value == 42);
		}
	}
	SECTION("variant& operator=(T&&) noexcept(see below) > performs_assignment") {
		using namespace RuntimeHelpers;
		{
			using V = plg::variant<ThrowsCtorT>;
			V v;
			v = 42;
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v).value == 42);
		}
		{
			using V = plg::variant<ThrowsCtorT, std::string>;
			V v;
			v = 42;
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v).value == 42);
		}
		{
			using V = plg::variant<ThrowsAssignT>;
			V v(100);
			try {
				v = 42;
				REQUIRE(false);
			} catch (...) { /* ... */
			}
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v).value == 100);
		}
		{
			using V = plg::variant<std::string, ThrowsAssignT>;
			V v(100);
			try {
				v = 42;
				REQUIRE(false);
			} catch (...) { /* ... */
			}
			REQUIRE(v.index() == 1);
			REQUIRE(plg::get<1>(v).value == 100);
		}
	}
#endif
	SECTION("variant& operator=(T&&) noexcept(see below) > noexcept") {
		using namespace MetaHelpers;
		{
			using V = plg::variant<Dummy, NoThrowT>;
			static_assert(std::is_nothrow_assignable<V, int>::value);
		}
		{
			using V = plg::variant<Dummy, ThrowsCtorT>;
			static_assert(!std::is_nothrow_assignable<V, int>::value);
		}
		{
			using V = plg::variant<Dummy, ThrowsAssignT>;
			static_assert(!std::is_nothrow_assignable<V, int>::value);
		}
	}
	SECTION("variant& operator=(T&&) noexcept(see below) > sfinae") {
		{
			using V = plg::variant<long, long long>;
			static_assert(!std::is_assignable<V, int>::value, "ambiguous");
		}
		{
			using V = plg::variant<std::string, std::string>;
			static_assert(!std::is_assignable<V, const char*>::value, "ambiguous");
		}
		{
			using V = plg::variant<std::string, void*>;
			static_assert(!std::is_assignable<V, int>::value, "no matching operator=");
		}
		{
			using V = plg::variant<std::string, float>;
			static_assert(std::is_assignable<V, int>::value == VariantAllowsNarrowingConversions,
						  "no matching operator=");
		}
		{
			using V = plg::variant<std::unique_ptr<int>, bool>;
			static_assert(!std::is_assignable<V, std::unique_ptr<char>>::value,
						  "no explicit bool in operator=");
			struct X {
				operator void*();
			};
			static_assert(!std::is_assignable<V, X>::value,
						  "no boolean conversion in operator=");
			// plg note : unlike libc++ implementation, we allow some boolean conversion
			/* static_assert(!std::is_assignable<V, std::false_type>::value,
					  "no converted to bool in operator="); */
		}
		{
			struct X {};
			struct Y {
				operator X();
			};
			using V = plg::variant<X>;
			static_assert(std::is_assignable<V, Y>::value,
						  "regression on user-defined conversions in operator=");
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int, int&&>;
			static_assert(!std::is_assignable<V, int>::value, "ambiguous");
		}
		{
			using V = plg::variant<int, const int&>;
			static_assert(!std::is_assignable<V, int>::value, "ambiguous");
		}
#endif // TEST_VARIANT_HAS_NO_REFERENCES
	}
}