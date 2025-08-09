#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

namespace {
	struct NoCtors {
		explicit NoCtors() noexcept = delete;
		explicit NoCtors(NoCtors const&) noexcept = delete;
		NoCtors& operator=(NoCtors const&) noexcept = delete;
		~NoCtors() = default;
	};

	template<class Var, class T, class... Args>
	constexpr auto test_emplace_exists_imp(int) -> decltype(std::declval<Var>().template emplace<T>(std::declval<Args>()...), true) {
		return true;
	}

	template<class, class, class...>
	constexpr auto test_emplace_exists_imp(long) -> bool {
		return false;
	}

	template<class... Args>
	constexpr bool emplace_exists() {
		return test_emplace_exists_imp<Args...>(0);
	}
} // namespace

TEST_CASE("variant > emplace_type_args", "[variant]") {
	SECTION("emplace_sfinae()") {
		{
			using V = plg::variant<int, void*, const void*, NoCtors>;
			static_assert(emplace_exists<V, int>());
			static_assert(emplace_exists<V, int, int>());
			static_assert(!emplace_exists<V, int, decltype(nullptr)>(), "cannot construct");
			static_assert(emplace_exists<V, void*, decltype(nullptr)>());
			static_assert(!emplace_exists<V, void*, int>(), "cannot construct");
			static_assert(emplace_exists<V, void*, int*>());
			static_assert(!emplace_exists<V, void*, const int*>());
			static_assert(emplace_exists<V, const void*, const int*>());
			static_assert(emplace_exists<V, const void*, int*>());
			static_assert(!emplace_exists<V, NoCtors>(), "cannot construct");
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int, int&, const int&, int&&, long, long, NoCtors>;
			static_assert(emplace_exists<V, int>());
			static_assert(emplace_exists<V, int, int>());
			static_assert(emplace_exists<V, int, long long>());
			static_assert(!emplace_exists<V, int, int, int>(), "too many args");
			static_assert(emplace_exists<V, int&, int&>());
			static_assert(!emplace_exists<V, int&>(), "cannot default construct ref");
			static_assert(!emplace_exists<V, int&, const int&>(), "cannot bind ref");
			static_assert(!emplace_exists<V, int&, int&&>(), "cannot bind ref");
			static_assert(emplace_exists<V, const int&, int&>());
			static_assert(emplace_exists<V, const int&, const int&>());
			static_assert(emplace_exists<V, const int&, int&&>());
			static_assert(!emplace_exists<V, const int&, void*>(), "not constructible from void*");
			static_assert(emplace_exists<V, int&&, int>());
			static_assert(!emplace_exists<V, int&&, int&>(), "cannot bind ref");
			static_assert(!emplace_exists<V, int&&, const int&>(), "cannot bind ref");
			static_assert(!emplace_exists<V, int&&, const int&&>(), "cannot bind ref");
			static_assert(!emplace_exists<V, long, long>(), "ambiguous");
			static_assert(!emplace_exists<V, NoCtors>(), "cannot construct void");
		}
#endif
	}
	SECTION("T& emplace(Args&&... args)") {
		{
			using V = plg::variant<int>;
			V v(42);
			auto& ref1 = v.emplace<int>();
			static_assert(std::is_same_v<int&, decltype(ref1)>);
			REQUIRE(plg::get<0>(v) == 0);
			REQUIRE(&ref1 == &plg::get<0>(v));
			auto& ref2 = v.emplace<int>(42);
			static_assert(std::is_same_v<int&, decltype(ref2)>);
			REQUIRE(plg::get<0>(v) == 42);
			REQUIRE(&ref2 == &plg::get<0>(v));
		}
		{
			using V = plg::variant<int, long, const void*, NoCtors, std::string>;
			const int x = 100;
			V v(plg::in_place_type<int>, -1);
			// default emplace a value
			auto& ref1 = v.emplace<long>();
			static_assert(std::is_same_v<long&, decltype(ref1)>);
			REQUIRE(plg::get<1>(v) == 0);
			REQUIRE(&ref1 == &plg::get<1>(v));
			auto& ref2 = v.emplace<const void*>(&x);
			static_assert(std::is_same_v<const void*&, decltype(ref2)>);
			REQUIRE(plg::get<2>(v) == &x);
			REQUIRE(&ref2 == &plg::get<2>(v));
			// emplace with multiple args
			auto& ref3 = v.emplace<std::string>(std::string(3, 'a'));
			static_assert(std::is_same_v<std::string&, decltype(ref3)>);
			REQUIRE(plg::get<4>(v) == "aaa");
			REQUIRE(&ref3 == &plg::get<4>(v));
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int, long, const int&, int&&, NoCtors, std::string>;
			const int x = 100;
			int y = 42;
			int z = 43;
			V v(plg::in_place_index<0>, -1);
			// default emplace a value
			auto& ref1 = v.emplace<long>();
			static_assert(std::is_same_v<long&, decltype(ref1)>);
			REQUIRE(plg::get<long>(v) == 0);
			REQUIRE(&ref1 == &plg::get<long>(v));
			// emplace a reference
			auto& ref2 = v.emplace<const int&>(x);
			static_assert(std::is_same_v<const int&, decltype(ref2)>);
			REQUIRE(&plg::get<const int&>(v) == &x);
			REQUIRE(&ref2 == &plg::get<const int&>(v));
			// emplace an rvalue reference
			auto& ref3 = v.emplace<int&&>(std::move(y));
			static_assert(std::is_same_v<int&&, decltype(ref3)>);
			REQUIRE(&plg::get<int&&>(v) == &y);
			REQUIRE(&ref3 == &plg::get<int&&>(v));
			// re-emplace a new reference over the active member
			auto& ref4 = v.emplace<int&&>(std::move(z));
			static_assert(std::is_same_v<int&, decltype(ref4)>);
			REQUIRE(&plg::get<int&&>(v) == &z);
			REQUIRE(&ref4 == &plg::get<int&&>(v));
			// emplace with multiple args
			auto& ref5 = v.emplace<std::string>(3u, 'a');
			static_assert(std::is_same_v<std::string&, decltype(ref5)>);
			REQUIRE(plg::get<std::string>(v) == "aaa");
			REQUIRE((void*)&ref5 == (void*)&plg::get<std::string>(v));
		}
#endif
	}
}
