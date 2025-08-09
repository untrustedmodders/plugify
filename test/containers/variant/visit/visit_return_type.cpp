#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

#include "app/variant_tester.hpp"

namespace {
	template<typename ReturnType>
	void test_call_operator_forwarding() {
		using Fn = ForwardingCallObject;
		Fn obj{};
		const Fn& cobj = obj;
		{// test call operator forwarding - no variant
			plg::visit<ReturnType>(obj);
			REQUIRE(Fn::check_call<>(CT_NonConst | CT_LValue));
			plg::visit<ReturnType>(cobj);
			REQUIRE(Fn::check_call<>(CT_Const | CT_LValue));
			plg::visit<ReturnType>(std::move(obj));
			REQUIRE(Fn::check_call<>(CT_NonConst | CT_RValue));
			plg::visit<ReturnType>(std::move(cobj));
			REQUIRE(Fn::check_call<>(CT_Const | CT_RValue));
		}
		{// test call operator forwarding - single variant, single arg
			using V = plg::variant<int>;
			V v(42);
			plg::visit<ReturnType>(obj, v);
			REQUIRE(Fn::check_call<int&>(CT_NonConst | CT_LValue));
			plg::visit<ReturnType>(cobj, v);
			REQUIRE(Fn::check_call<int&>(CT_Const | CT_LValue));
			plg::visit<ReturnType>(std::move(obj), v);
			REQUIRE(Fn::check_call<int&>(CT_NonConst | CT_RValue));
			plg::visit<ReturnType>(std::move(cobj), v);
			REQUIRE(Fn::check_call<int&>(CT_Const | CT_RValue));
		}
		{// test call operator forwarding - single variant, multi arg
			using V = plg::variant<int, long, double>;
			V v(42l);
			plg::visit<ReturnType>(obj, v);
			REQUIRE(Fn::check_call<long&>(CT_NonConst | CT_LValue));
			plg::visit<ReturnType>(cobj, v);
			REQUIRE(Fn::check_call<long&>(CT_Const | CT_LValue));
			plg::visit<ReturnType>(std::move(obj), v);
			REQUIRE(Fn::check_call<long&>(CT_NonConst | CT_RValue));
			plg::visit<ReturnType>(std::move(cobj), v);
			REQUIRE(Fn::check_call<long&>(CT_Const | CT_RValue));
		}
		{// test call operator forwarding - multi variant, multi arg
			using V = plg::variant<int, long, double>;
			using V2 = plg::variant<int*, std::string>;
			V v(42l);
			V2 v2("hello");
			plg::visit<int>(obj, v, v2);
			assert((Fn::check_call<long&, std::string&>(CT_NonConst | CT_LValue)));
			plg::visit<ReturnType>(cobj, v, v2);
			REQUIRE((Fn::check_call<long&, std::string&>(CT_Const | CT_LValue)));
			plg::visit<ReturnType>(std::move(obj), v, v2);
			REQUIRE((Fn::check_call<long&, std::string&>(CT_NonConst | CT_RValue)));
			plg::visit<ReturnType>(std::move(cobj), v, v2);
			REQUIRE((Fn::check_call<long&, std::string&>(CT_Const | CT_RValue)));
		}
		{
			using V = plg::variant<int, long, double, std::string>;
			V v1(42l), v2("hello"), v3(101), v4(1.1);
			plg::visit<ReturnType>(obj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int&, double&>(CT_NonConst | CT_LValue)));
			plg::visit<ReturnType>(cobj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int&, double&>(CT_Const | CT_LValue)));
			plg::visit<ReturnType>(std::move(obj), v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int&, double&>(CT_NonConst | CT_RValue)));
			plg::visit<ReturnType>(std::move(cobj), v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int&, double&>(CT_Const | CT_RValue)));
		}
		{
			using V = plg::variant<int, long, double, int*, std::string>;
			V v1(42l), v2("hello"), v3(nullptr), v4(1.1);
			plg::visit<ReturnType>(obj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int*&, double&>(CT_NonConst | CT_LValue)));
			plg::visit<ReturnType>(cobj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int*&, double&>(CT_Const | CT_LValue)));
			plg::visit<ReturnType>(std::move(obj), v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int*&, double&>(CT_NonConst | CT_RValue)));
			plg::visit<ReturnType>(std::move(cobj), v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int*&, double&>(CT_Const | CT_RValue)));
		}
	}

	template<typename ReturnType>
	void test_argument_forwarding() {
		using Fn = ForwardingCallObject;
		Fn obj{};
		const auto Val = CT_LValue | CT_NonConst;
		{// single argument - value type
			using V = plg::variant<int>;
			V v(42);
			const V& cv = v;
			plg::visit<ReturnType>(obj, v);
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit<ReturnType>(obj, cv);
			REQUIRE(Fn::check_call<const int&>(Val));
			plg::visit<ReturnType>(obj, std::move(v));
			REQUIRE(Fn::check_call<int&&>(Val));
			plg::visit<ReturnType>(obj, std::move(cv));
			REQUIRE(Fn::check_call<const int&&>(Val));
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{// single argument - lvalue reference
			using V = plg::variant<int&>;
			int x = 42;
			V v(x);
			const V& cv = v;
			plg::visit<ReturnType>(obj, v);
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit<ReturnType>(obj, cv);
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit<ReturnType>(obj, std::move(v));
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit<ReturnType>(obj, std::move(cv));
			REQUIRE(Fn::check_call<int&>(Val));
		}
		{// single argument - rvalue reference
			using V = plg::variant<int&&>;
			int x = 42;
			V v(std::move(x));
			const V& cv = v;
			plg::visit<ReturnType>(obj, v);
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit<ReturnType>(obj, cv);
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit<ReturnType>(obj, std::move(v));
			REQUIRE(Fn::check_call<int&&>(Val));
			plg::visit<ReturnType>(obj, std::move(cv));
			REQUIRE(Fn::check_call<int&&>(Val));
		}
#endif
		{// multi argument - multi variant
			using V = plg::variant<int, std::string, long>;
			V v1(42), v2("hello"), v3(43l);
			plg::visit<ReturnType>(obj, v1, v2, v3);
			REQUIRE((Fn::check_call<int&, std::string&, long&>(Val)));
			plg::visit<ReturnType>(obj, std::as_const(v1), std::as_const(v2), std::move(v3));
			REQUIRE((Fn::check_call<const int&, const std::string&, long&&>(Val)));
		}
		{
			using V = plg::variant<int, long, double, std::string>;
			V v1(42l), v2("hello"), v3(101), v4(1.1);
			plg::visit<ReturnType>(obj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int&, double&>(Val)));
			plg::visit<ReturnType>(obj, std::as_const(v1), std::as_const(v2), std::move(v3), std::move(v4));
			REQUIRE((Fn::check_call<const long&, const std::string&, int&&, double&&>(Val)));
		}
		{
			using V = plg::variant<int, long, double, int*, std::string>;
			V v1(42l), v2("hello"), v3(nullptr), v4(1.1);
			plg::visit<ReturnType>(obj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int*&, double&>(Val)));
			plg::visit<ReturnType>(obj, std::as_const(v1), std::as_const(v2), std::move(v3), std::move(v4));
			REQUIRE((Fn::check_call<const long&, const std::string&, int*&&, double&&>(Val)));
		}
	}

	template<typename ReturnType>
	void test_return_type() {
		using Fn = ForwardingCallObject;
		Fn obj{};
		const Fn& cobj = obj;
		{// test call operator forwarding - no variant
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(obj)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(cobj)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(obj))), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(cobj))), ReturnType>);
		}
		{// test call operator forwarding - single variant, single arg
			using V = plg::variant<int>;
			V v(42);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(obj, v)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(cobj, v)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(obj), v)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(cobj), v)), ReturnType>);
		}
		{// test call operator forwarding - single variant, multi arg
			using V = plg::variant<int, long, double>;
			V v(42l);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(obj, v)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(cobj, v)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(obj), v)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(cobj), v)), ReturnType>);
		}
		{// test call operator forwarding - multi variant, multi arg
			using V = plg::variant<int, long, double>;
			using V2 = plg::variant<int*, std::string>;
			V v(42l);
			V2 v2("hello");
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(obj, v, v2)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(cobj, v, v2)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(obj), v, v2)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(cobj), v, v2)), ReturnType>);
		}
		{
			using V = plg::variant<int, long, double, std::string>;
			V v1(42l), v2("hello"), v3(101), v4(1.1);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(obj, v1, v2, v3, v4)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(cobj, v1, v2, v3, v4)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(obj), v1, v2, v3, v4)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(cobj), v1, v2, v3, v4)), ReturnType>);
		}
		{
			using V = plg::variant<int, long, double, int*, std::string>;
			V v1(42l), v2("hello"), v3(nullptr), v4(1.1);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(obj, v1, v2, v3, v4)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(cobj, v1, v2, v3, v4)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(obj), v1, v2, v3, v4)), ReturnType>);
			static_assert(std::is_same_v<decltype(plg::visit<ReturnType>(std::move(cobj), v1, v2, v3, v4)), ReturnType>);
		}
	}

	void test_constexpr_void() {
		constexpr ReturnFirst obj{};
		constexpr ReturnArity aobj{};
		{
			using V = plg::variant<int>;
			constexpr V v(42);
			static_assert((plg::visit<void>(obj, v), 42) == 42);
		}
		{
			using V = plg::variant<short, long, char>;
			constexpr V v(42l);
			static_assert((plg::visit<void>(obj, v), 42) == 42);
		}
		{
			using V1 = plg::variant<int>;
			using V2 = plg::variant<int, char*, long long>;
			using V3 = plg::variant<bool, int, int>;
			constexpr V1 v1;
			constexpr V2 v2(nullptr);
			constexpr V3 v3;
			static_assert((plg::visit<void>(aobj, v1, v2, v3), 3) == 3);
		}
		{
			using V1 = plg::variant<int>;
			using V2 = plg::variant<int, char*, long long>;
			using V3 = plg::variant<void*, int, int>;
			constexpr V1 v1;
			constexpr V2 v2(nullptr);
			constexpr V3 v3;
			static_assert((plg::visit<void>(aobj, v1, v2, v3), 3) == 3);
		}
		{
			using V = plg::variant<int, long, double, int*>;
			constexpr V v1(42l), v2(101), v3(nullptr), v4(1.1);
			static_assert((plg::visit<void>(aobj, v1, v2, v3, v4), 4) == 4);
		}
		{
			using V = plg::variant<int, long, double, long long, int*>;
			constexpr V v1(42l), v2(101), v3(nullptr), v4(1.1);
			static_assert((plg::visit<void>(aobj, v1, v2, v3, v4), 4) == 4);
		}
	}

	void test_constexpr_int() {
		constexpr ReturnFirst obj{};
		constexpr ReturnArity aobj{};
		{
			using V = plg::variant<int>;
			constexpr V v(42);
			static_assert(plg::visit<int>(obj, v) == 42);
		}
		{
			using V = plg::variant<short, long, char>;
			constexpr V v(42l);
			static_assert(plg::visit<int>(obj, v) == 42);
		}
		{
			using V1 = plg::variant<int>;
			using V2 = plg::variant<int, char*, long long>;
			using V3 = plg::variant<bool, int, int>;
			constexpr V1 v1;
			constexpr V2 v2(nullptr);
			constexpr V3 v3;
			static_assert(plg::visit<int>(aobj, v1, v2, v3) == 3);
		}
		{
			using V1 = plg::variant<int>;
			using V2 = plg::variant<int, char*, long long>;
			using V3 = plg::variant<void*, int, int>;
			constexpr V1 v1;
			constexpr V2 v2(nullptr);
			constexpr V3 v3;
			static_assert(plg::visit<int>(aobj, v1, v2, v3) == 3);
		}
		{
			using V = plg::variant<int, long, double, int*>;
			constexpr V v1(42l), v2(101), v3(nullptr), v4(1.1);
			static_assert(plg::visit<int>(aobj, v1, v2, v3, v4) == 4);
		}
		{
			using V = plg::variant<int, long, double, long long, int*>;
			constexpr V v1(42l), v2(101), v3(nullptr), v4(1.1);
			static_assert(plg::visit<int>(aobj, v1, v2, v3, v4) == 4);
		}
	}

	template<typename ReturnType>
	void test_exceptions() {
#if TEST_HAS_NO_EXCEPTIONS
		ReturnArity obj{};
		auto test = [&](auto&&... args) {
			try {
				plg::visit<ReturnType>(obj, args...);
			} catch (const plg::bad_variant_access&) {
				return true;
			} catch (...) {
			}
			return false;
		};
		{
			using V = plg::variant<int, MakeEmptyT>;
			V v;
			makeEmpty(v);
			REQUIRE(test(v));
		}
		{
			using V = plg::variant<int, MakeEmptyT>;
			using V2 = plg::variant<long, std::string, void*>;
			V v;
			makeEmpty(v);
			V2 v2("hello");
			REQUIRE(test(v, v2));
		}
		{
			using V = plg::variant<int, MakeEmptyT>;
			using V2 = plg::variant<long, std::string, void*>;
			V v;
			makeEmpty(v);
			V2 v2("hello");
			REQUIRE(test(v2, v));
		}
		{
			using V = plg::variant<int, MakeEmptyT>;
			using V2 = plg::variant<long, std::string, void*, MakeEmptyT>;
			V v;
			makeEmpty(v);
			V2 v2;
			makeEmpty(v2);
			REQUIRE(test(v, v2));
		}
		{
			using V = plg::variant<int, long, double, MakeEmptyT>;
			V v1(42l), v2(101), v3(202), v4(1.1);
			makeEmpty(v1);
			REQUIRE(test(v1, v2, v3, v4));
		}
		{
			using V = plg::variant<int, long, double, long long, MakeEmptyT>;
			V v1(42l), v2(101), v3(202), v4(1.1);
			makeEmpty(v1);
			makeEmpty(v2);
			makeEmpty(v3);
			makeEmpty(v4);
			REQUIRE(test(v1, v2, v3, v4));
		}
#endif // TEST_HAS_NO_EXCEPTIONS
	}

	// See https://bugs.llvm.org/show_bug.cgi?id=31916
	template<typename ReturnType>
	void test_caller_accepts_nonconst() {
		struct A {};
		struct Visitor {
			auto operator()(A&) {
				if constexpr (!std::is_void_v<ReturnType>) {
					return ReturnType{};
				}
			}
		};
		plg::variant<A> v;
		plg::visit<ReturnType>(Visitor{}, v);
	}

	void test_constexpr_explicit_side_effect() {
		auto test_lambda = [](int arg) constexpr {
			plg::variant<int> v = 101;
			plg::visit<void>([arg](int& x) constexpr { x = arg; }, v);
			return plg::get<int>(v);
		};

		static_assert(test_lambda(202) == 202);
	}
} // namespace

TEST_CASE("variant > visit > with return", "[variant]") {
	SECTION("R visit(Visitor&& vis, Variants&&... vars)") {
		//test_call_operator_forwarding<void>();
		test_argument_forwarding<void>();
		test_return_type<void>();
		test_constexpr_void();
		test_exceptions<void>();
		test_caller_accepts_nonconst<void>();
		//test_call_operator_forwarding<int>();
		test_argument_forwarding<int>();
		test_return_type<int>();
		test_constexpr_int();
		test_exceptions<int>();
		test_caller_accepts_nonconst<int>();
		test_constexpr_explicit_side_effect();
	}
}
