#include <catch_amalgamated.hpp>

#include <plugifyvariant.hpp>

namespace {
	struct NoCtors {
		explicit NoCtors() noexcept = delete;
		explicit NoCtors(NoCtors const&) noexcept = delete;
		NoCtors& operator=(NoCtors const&) noexcept = delete;
		~NoCtors() = default;
	};

	struct InitList {
		std::size_t size;
		constexpr InitList(std::initializer_list<int> il) : size(il.size()) {}
	};

	struct InitListArg {
		std::size_t size;
		int value;
		constexpr InitListArg(std::initializer_list<int> il, int v)
			: size(il.size()), value(v) {}
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

TEST_CASE("variant > emplace_index_init_list_args", "[variant]") {
	SECTION("emplace_sfinae()") {
		{
			using V = plg::variant<int, NoCtors, InitList, InitListArg, long, long>;
			using IL = std::initializer_list<int>;
			static_assert(!emplace_exists<V, 1, IL>(), "no such constructor");
			static_assert(emplace_exists<V, 2, IL>());
			static_assert(!emplace_exists<V, 2, int>(), "args don't match");
			static_assert(!emplace_exists<V, 2, IL, int>(), "too many args");
			static_assert(emplace_exists<V, 3, IL, int>());
			static_assert(!emplace_exists<V, 3, int>(), "args don't match");
			static_assert(!emplace_exists<V, 3, IL>(), "too few args");
			static_assert(!emplace_exists<V, 3, IL, int, int>(), "too many args");
		}
	}
	SECTION("variant_alternative_t<I, variant<Types...>>& emplace(initializer_list<U> il,Args&&... args)") {
		using V = plg::variant<int, InitList, InitListArg, NoCtors>;
		V v;
		auto& ref1 = v.emplace<1>({1, 2, 3});
		static_assert(std::is_same_v<InitList&, decltype(ref1)>);
		REQUIRE(plg::get<1>(v).size == 3);
		REQUIRE(&ref1 == &plg::get<1>(v));
		auto& ref2 = v.emplace<2>({1, 2, 3, 4}, 42);
		static_assert(std::is_same_v<InitListArg&, decltype(ref2)>);
		REQUIRE(plg::get<2>(v).size == 4);
		REQUIRE(plg::get<2>(v).value == 42);
		REQUIRE(&ref2 == &plg::get<2>(v));
		auto& ref3 = v.emplace<1>({1});
		static_assert(std::is_same_v<InitList&, decltype(ref3)>);
		REQUIRE(plg::get<1>(v).size == 1);
		REQUIRE(&ref3 == &plg::get<1>(v));
	}
}
