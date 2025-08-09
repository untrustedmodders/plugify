#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

namespace {
	struct Incomplete;
	template<class T>
	struct Holder {
		T t;
	};

	constexpr bool test(bool do_it) {
		if (do_it) {
			plg::variant<Holder<Incomplete>*, int> v = nullptr;
			plg::visit([](auto) {}, v);
			plg::visit([](auto) -> Holder<Incomplete>* { return nullptr; }, v);
			plg::visit<void>([](auto) {}, v);
			plg::visit<void*>([](auto) -> Holder<Incomplete>* { return nullptr; }, v);
		}
		return true;
	}
} // namespace

TEST_CASE("variant > visit", "[variant]") {
	SECTION("decltype(auto) visit(Visitor&& vis, Variants&&... vars)") {
		test(true);
		static_assert(test(true));
	}
}
