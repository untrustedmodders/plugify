#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

#include "app/variant_tester.hpp"

namespace {
	struct Dummy {
		Dummy() = default;
	};

	struct ThrowsT {
		ThrowsT(int) noexcept(false) {}
	};

	struct NoThrowT {
		NoThrowT(int) noexcept(true) {}
	};

	struct AnyConstructible {
		template<typename T>
		AnyConstructible(T&&) {}
	};
	struct NoConstructible {
		NoConstructible() = delete;
	};
	template<class T>
	struct RValueConvertibleFrom {
		RValueConvertibleFrom(T&&) {}
	};

	struct BoomOnAnything {
		template<class T>
		constexpr BoomOnAnything(T) { static_assert(!std::is_same<T, T>::value); }
	};

	struct Bar {};
	struct Baz {};
} // namespace

TEST_CASE("variant constructor<T>", "[variant]") {
	SECTION("variant(T&&) noexcept(see below) > basic") {
		{
			constexpr plg::variant<int> v(42);
			static_assert(v.index() == 0);
			static_assert(plg::get<0>(v) == 42);
		}
		{
			constexpr plg::variant<int, long> v(42l);
			static_assert(v.index() == 1);
			static_assert(plg::get<1>(v) == 42);
		}
		{
			constexpr plg::variant<unsigned, long> v(42);
			static_assert(v.index() == 1);
			static_assert(plg::get<1>(v) == 42);
		}
		{
			plg::variant<std::string, bool const> v = "foo";
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v) == "foo");
		}
		{
			plg::variant<bool volatile, std::unique_ptr<int>> v = nullptr;
			REQUIRE(v.index() == 1);
			REQUIRE(plg::get<1>(v) == nullptr);
		}
		{
			plg::variant<bool volatile const, int> v = true;
			REQUIRE(v.index() == 0);
			REQUIRE(plg::get<0>(v));
		}
		{
			plg::variant<RValueConvertibleFrom<int>> v1 = 42;
			REQUIRE(v1.index() == 0);

			int x = 42;
			plg::variant<RValueConvertibleFrom<int>, AnyConstructible> v2 = x;
			REQUIRE(v2.index() == 1);
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<const int&, int&&, long>;
			//static_assert(std::is_convertible<int&, V>::value, "must be implicit");
			int x = 42;
			V v(x);
			REQUIRE(v.index() == 0);
			REQUIRE(&plg::get<0>(v) == &x);
		}
		{
			using V = plg::variant<const int&, int&&, long>;
			static_assert(std::is_convertible<int, V>::value, "must be implicit");
			int x = 42;
			V v(std::move(x));
			REQUIRE(v.index() == 1);
			REQUIRE(&plg::get<1>(v) == &x);
		}
#endif
	}
	SECTION("variant(T&&) noexcept(see below) > noexcept") {
		{
			using V = plg::variant<Dummy, NoThrowT>;
			static_assert(std::is_nothrow_constructible<V, int>::value);
		}
		{
			using V = plg::variant<Dummy, ThrowsT>;
			static_assert(!std::is_nothrow_constructible<V, int>::value);
		}
	}
	SECTION("variant(T&&) noexcept(see below) > sfinae") {
		{
			using V = plg::variant<long, long long>;
			static_assert(!std::is_constructible<V, int>::value, "ambiguous");
		}
		{
			using V = plg::variant<std::string, std::string>;
			static_assert(!std::is_constructible<V, const char*>::value, "ambiguous");
		}
		{
			using V = plg::variant<std::string, void*>;
			static_assert(!std::is_constructible<V, int>::value,
						  "no matching constructor");
		}
		{
			using V = plg::variant<std::string, float>;
			static_assert(std::is_constructible<V, int>::value == VariantAllowsNarrowingConversions,
						  "no matching constructor");
		}
		{
			using V = plg::variant<std::unique_ptr<int>, bool>;
			static_assert(!std::is_constructible<V, std::unique_ptr<char>>::value,
						  "no explicit bool in constructor");
			struct X {
				operator void*();
			};
			static_assert(!std::is_constructible<V, X>::value,
						  "no boolean conversion in constructor");
			/* static_assert(!std::is_constructible<V, std::false_type>::value,
                  "no converted to bool in constructor"); */
		}
		{
			struct X {};
			struct Y {
				operator X();
			};
			using V = plg::variant<X>;
			static_assert(std::is_constructible<V, Y>::value,
						  "regression on user-defined conversions in constructor");
		}
		{
			using V = plg::variant<AnyConstructible, NoConstructible>;
			static_assert(!std::is_constructible<V, plg::in_place_type_t<NoConstructible>>::value, "no matching constructor");
			static_assert(!std::is_constructible<V, plg::in_place_index_t<1>>::value, "no matching constructor");
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int, int&&>;
			static_assert(!std::is_constructible<V, int>::value, "ambiguous");
		}
		{
			using V = plg::variant<int, const int&>;
			static_assert(!std::is_constructible<V, int>::value, "ambiguous");
		}
#endif
	}
	SECTION("variant(T&&) noexcept(see below) > no_narrowing_check_for_class_types") {
		using V = plg::variant<int, BoomOnAnything>;
		V v(42);
		REQUIRE(v.index() == 0);
		REQUIRE(plg::get<0>(v) == 42);
	}
	SECTION("variant(T&&) noexcept(see below) > construction_with_repeated_types") {
		using V = plg::variant<int, Bar, Baz, int, Baz, int, int>;
		static_assert(!std::is_constructible<V, int>::value);
		static_assert(!std::is_constructible<V, Baz>::value);
		// OK, the selected type appears only once and so it shouldn't
		// be affected by the duplicate types.
		static_assert(std::is_constructible<V, Bar>::value);
	}
}
