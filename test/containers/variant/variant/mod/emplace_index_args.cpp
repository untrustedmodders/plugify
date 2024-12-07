#include <catch_amalgamated.hpp>

#include <plugifyvariant.hpp>

namespace {
	struct NoCtors {
		explicit NoCtors() noexcept = delete;
		explicit NoCtors(NoCtors const&) noexcept = delete;
		NoCtors& operator=(NoCtors const&) noexcept = delete;
		~NoCtors() = default;
	};

	template<class Var, size_t I, class... Args>
	constexpr auto test_emplace_exists_imp(int) -> decltype(std::declval<Var>().template emplace<I>(std::declval<Args>()...), true) {
		return true;
	}

	template<class, size_t, class...>
	constexpr auto test_emplace_exists_imp(long) -> bool {
		return false;
	}

	template<class Var, size_t I, class... Args>
	constexpr bool emplace_exists() {
		return test_emplace_exists_imp<Var, I, Args...>(0);
	}
} // namespace

TEST_CASE("variant > emplace_index_args", "[variant]") {
	SECTION("emplace_sfinae()") {
		{
			using V = plg::variant<int, void*, const void*, NoCtors>;
			static_assert(emplace_exists<V, 0>());
			static_assert(emplace_exists<V, 0, int>());
			static_assert(!emplace_exists<V, 0, decltype(nullptr)>(), "cannot construct");
			static_assert(emplace_exists<V, 1, decltype(nullptr)>());
			static_assert(emplace_exists<V, 1, int*>());
			static_assert(!emplace_exists<V, 1, const int*>());
			static_assert(!emplace_exists<V, 1, int>(), "cannot construct");
			static_assert(emplace_exists<V, 2, const int*>());
			static_assert(emplace_exists<V, 2, int*>());
			static_assert(!emplace_exists<V, 3>(), "cannot construct");
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int, int&, const int&, int&&, NoCtors>;
			static_assert(emplace_exists<V, 0>());
			static_assert(emplace_exists<V, 0, int>());
			static_assert(emplace_exists<V, 0, long long>());
			static_assert(!emplace_exists<V, 0, int, int>(), "too many args");
			static_assert(emplace_exists<V, 1, int&>());
			static_assert(!emplace_exists<V, 1>(), "cannot default construct ref");
			static_assert(!emplace_exists<V, 1, const int&>(), "cannot bind ref");
			static_assert(!emplace_exists<V, 1, int&&>(), "cannot bind ref");
			static_assert(emplace_exists<V, 2, int&>());
			static_assert(emplace_exists<V, 2, const int&>());
			static_assert(emplace_exists<V, 2, int&&>());
			static_assert(!emplace_exists<V, 2, void*>(), "not constructible from void*");
			static_assert(emplace_exists<V, 3, int>());
			static_assert(!emplace_exists<V, 3, int&>(), "cannot bind ref");
			static_assert(!emplace_exists<V, 3, const int&>(), "cannot bind ref");
			static_assert(!emplace_exists<V, 3, const int&&>(), "cannot bind ref");
			static_assert(!emplace_exists<V, 4>(), "no ctors");
		}
#endif
	}
	SECTION("variant_alternative_t<I, variant<Types...>>& emplace(Args&&... args)") {
		{
			using V = plg::variant<int>;
			V v(42);
			auto& ref1 = v.emplace<0>();
			static_assert(std::is_same_v<int&, decltype(ref1)>);
			REQUIRE(plg::get<0>(v) == 0);
			REQUIRE(&ref1 == &plg::get<0>(v));
			auto& ref2 = v.emplace<0>(42);
			static_assert(std::is_same_v<int&, decltype(ref2)>);
			REQUIRE(plg::get<0>(v) == 42);
			REQUIRE(&ref2 == &plg::get<0>(v));
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{
			using V = plg::variant<int, long, const void*, NoCtors, std::string>;
			const int x = 100;
			V v(plg::in_place_index<0>, -1);
			// default emplace a value
			auto& ref1 = v.emplace<1>();
			static_assert(std::is_same_v<long&, decltype(ref1)>);
			REQUIRE(plg::get<1>(v) == 0);
			REQUIRE(&ref1 == &plg::get<1>(v));
			auto& ref2 = v.emplace<2>(&x);
			static_assert(std::is_same_v<const void*&, decltype(ref2)>);
			REQUIRE(plg::get<2>(v) == &x);
			REQUIRE(&ref2 == &plg::get<2>(v));
			// emplace with multiple args
			auto& ref3 = v.emplace<4>(3u, 'a');
			static_assert(std::is_same_v<std::string&, decltype(ref3)>);
			REQUIRE(plg::get<4>(v) == "aaa");
			REQUIRE(&ref3 == &plg::get<4>(v));
		}
		{
			using V = plg::variant<int, long, const int&, int&&, NoCtors, std::string>;
			const int x = 100;
			int y = 42;
			int z = 43;
			V v(plg::in_place_index<0>, -1);
			// default emplace a value
			auto& ref1 = v.emplace<1>();
			static_assert(std::is_same_v<long&, decltype(ref1)>);
			REQUIRE(plg::get<1>(v) == 0);
			REQUIRE(&ref1 == &plg::get<1>(v));
			// emplace a reference
			auto& ref2 = v.emplace<2>(x);
			static_assert(std::is_same_v<&, decltype(ref)>);
			REQUIRE(&plg::get<2>(v) == &x);
			REQUIRE(&ref2 == &plg::get<2>(v));
			// emplace an rvalue reference
			auto& ref3 = v.emplace<3>(std::move(y));
			static_assert(std::is_same_v<&, decltype(ref)>);
			REQUIRE(&plg::get<3>(v) == &y);
			REQUIRE(&ref3 == &plg::get<3>(v));
			// re-emplace a new reference over the active member
			auto& ref4 = v.emplace<3>(std::move(z));
			static_assert(std::is_same_v<&, decltype(ref)>);
			REQUIRE(&plg::get<3>(v) == &z);
			REQUIRE(&ref4 == &plg::get<3>(v));
			// emplace with multiple args
			auto& ref5 = v.emplace<5>(3u, 'a');
			static_assert(std::is_same_v<std::string&, decltype(ref5)>);
			REQUIRE(plg::get<5>(v) == "aaa");
			REQUIRE(&ref5 == &plg::get<5>(v));
		}
#endif
	}
}