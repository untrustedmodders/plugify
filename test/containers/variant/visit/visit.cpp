#include <catch_amalgamated.hpp>

#include <plg/variant.hpp>

#include "app/variant_tester.hpp"

namespace {
	void test_call_operator_forwarding() {
		using Fn = ForwardingCallObject;
		Fn obj{};
		const Fn& cobj = obj;
		{// test call operator forwarding - no variant
			plg::visit(obj);
			REQUIRE(Fn::check_call<>(CT_NonConst | CT_LValue));
			plg::visit(cobj);
			REQUIRE(Fn::check_call<>(CT_Const | CT_LValue));
			plg::visit(std::move(obj));
			REQUIRE(Fn::check_call<>(CT_NonConst | CT_RValue));
			plg::visit(std::move(cobj));
			REQUIRE(Fn::check_call<>(CT_Const | CT_RValue));
		}
		{// test call operator forwarding - single variant, single arg
			using V = plg::variant<int>;
			V v(42);
			plg::visit(obj, v);
			REQUIRE(Fn::check_call<int&>(CT_NonConst | CT_LValue));
			plg::visit(cobj, v);
			REQUIRE(Fn::check_call<int&>(CT_Const | CT_LValue));
			plg::visit(std::move(obj), v);
			REQUIRE(Fn::check_call<int&>(CT_NonConst | CT_RValue));
			plg::visit(std::move(cobj), v);
			REQUIRE(Fn::check_call<int&>(CT_Const | CT_RValue));
		}
		{// test call operator forwarding - single variant, multi arg
			using V = plg::variant<int, long, double>;
			V v(42l);
			plg::visit(obj, v);
			REQUIRE(Fn::check_call<long&>(CT_NonConst | CT_LValue));
			plg::visit(cobj, v);
			REQUIRE(Fn::check_call<long&>(CT_Const | CT_LValue));
			plg::visit(std::move(obj), v);
			REQUIRE(Fn::check_call<long&>(CT_NonConst | CT_RValue));
			plg::visit(std::move(cobj), v);
			REQUIRE(Fn::check_call<long&>(CT_Const | CT_RValue));
		}
		{// test call operator forwarding - multi variant, multi arg
			using V = plg::variant<int, long, double>;
			using V2 = plg::variant<int*, std::string>;
			V v(42l);
			V2 v2("hello");
			plg::visit(obj, v, v2);
			REQUIRE((Fn::check_call<long&, std::string&>(CT_NonConst | CT_LValue)));
			plg::visit(cobj, v, v2);
			REQUIRE((Fn::check_call<long&, std::string&>(CT_Const | CT_LValue)));
			plg::visit(std::move(obj), v, v2);
			REQUIRE((Fn::check_call<long&, std::string&>(CT_NonConst | CT_RValue)));
			plg::visit(std::move(cobj), v, v2);
			REQUIRE((Fn::check_call<long&, std::string&>(CT_Const | CT_RValue)));
		}
		{
			using V = plg::variant<int, long, double, std::string>;
			V v1(42l), v2("hello"), v3(101), v4(1.1);
			plg::visit(obj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int&, double&>(CT_NonConst | CT_LValue)));
			plg::visit(cobj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int&, double&>(CT_Const | CT_LValue)));
			plg::visit(std::move(obj), v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int&, double&>(CT_NonConst | CT_RValue)));
			plg::visit(std::move(cobj), v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int&, double&>(CT_Const | CT_RValue)));
		}
		{
			using V = plg::variant<int, long, double, int*, std::string>;
			V v1(42l), v2("hello"), v3(nullptr), v4(1.1);
			plg::visit(obj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int*&, double&>(CT_NonConst | CT_LValue)));
			plg::visit(cobj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int*&, double&>(CT_Const | CT_LValue)));
			plg::visit(std::move(obj), v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int*&, double&>(CT_NonConst | CT_RValue)));
			plg::visit(std::move(cobj), v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int*&, double&>(CT_Const | CT_RValue)));
		}
	}

	void test_argument_forwarding() {
		using Fn = ForwardingCallObject;
		Fn obj{};
		const auto Val = CT_LValue | CT_NonConst;
		{// single argument - value type
			using V = plg::variant<int>;
			V v(42);
			const V& cv = v;
			plg::visit(obj, v);
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit(obj, cv);
			REQUIRE(Fn::check_call<const int&>(Val));
			plg::visit(obj, std::move(v));
			REQUIRE(Fn::check_call<int&&>(Val));
			plg::visit(obj, std::move(cv));
			REQUIRE(Fn::check_call<const int&&>(Val));
		}
#if !TEST_VARIANT_HAS_NO_REFERENCES
		{// single argument - lvalue reference
			using V = plg::variant<int&>;
			int x = 42;
			V v(x);
			const V& cv = v;
			plg::visit(obj, v);
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit(obj, cv);
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit(obj, std::move(v));
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit(obj, std::move(cv));
			REQUIRE(Fn::check_call<int&>(Val));
		}
		{// single argument - rvalue reference
			using V = plg::variant<int&&>;
			int x = 42;
			V v(std::move(x));
			const V& cv = v;
			plg::visit(obj, v);
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit(obj, cv);
			REQUIRE(Fn::check_call<int&>(Val));
			plg::visit(obj, std::move(v));
			REQUIRE(Fn::check_call<int&&>(Val));
			plg::visit(obj, std::move(cv));
			REQUIRE(Fn::check_call<int&&>(Val));
		}
#endif
		{// multi argument - multi variant
			using V = plg::variant<int, std::string, long>;
			V v1(42), v2("hello"), v3(43l);
			plg::visit(obj, v1, v2, v3);
			REQUIRE((Fn::check_call<int&, std::string&, long&>(Val)));
			plg::visit(obj, std::as_const(v1), std::as_const(v2), std::move(v3));
			REQUIRE((Fn::check_call<const int&, const std::string&, long&&>(Val)));
		}
		{
			using V = plg::variant<int, long, double, std::string>;
			V v1(42l), v2("hello"), v3(101), v4(1.1);
			plg::visit(obj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int&, double&>(Val)));
			plg::visit(obj, std::as_const(v1), std::as_const(v2), std::move(v3), std::move(v4));
			REQUIRE((Fn::check_call<const long&, const std::string&, int&&, double&&>(Val)));
		}
		{
			using V = plg::variant<int, long, double, int*, std::string>;
			V v1(42l), v2("hello"), v3(nullptr), v4(1.1);
			plg::visit(obj, v1, v2, v3, v4);
			REQUIRE((Fn::check_call<long&, std::string&, int*&, double&>(Val)));
			plg::visit(obj, std::as_const(v1), std::as_const(v2), std::move(v3), std::move(v4));
			REQUIRE((Fn::check_call<const long&, const std::string&, int*&&, double&&>(Val)));
		}
	}

	void test_return_type() {
		using Fn = ForwardingCallObject;
		Fn obj{};
		const Fn& cobj = obj;
		{// test call operator forwarding - no variant
			static_assert(std::is_same_v<decltype(plg::visit(obj)), Fn&>);
			static_assert(std::is_same_v<decltype(plg::visit(cobj)), const Fn&>);
#ifndef _MSC_VER
			static_assert(std::is_same_v<decltype(plg::visit(std::move(obj))), Fn&&>);
			static_assert(std::is_same_v<decltype(plg::visit(std::move(cobj))), const Fn&&>);
#endif // _MSC_VER
		}
		{// test call operator forwarding - single variant, single arg
			using V = plg::variant<int>;
			V v(42);
			static_assert(std::is_same_v<decltype(plg::visit(obj, v)), Fn&>);
			static_assert(std::is_same_v<decltype(plg::visit(cobj, v)), const Fn&>);
#ifndef _MSC_VER
			static_assert(std::is_same_v<decltype(plg::visit(std::move(obj), v)), Fn&&>);
			static_assert(std::is_same_v<decltype(plg::visit(std::move(cobj), v)), const Fn&&>);
#endif // _MSC_VER
		}
		{// test call operator forwarding - single variant, multi arg
			using V = plg::variant<int, long, double>;
			V v(42l);
			static_assert(std::is_same_v<decltype(plg::visit(obj, v)), Fn&>);
			static_assert(std::is_same_v<decltype(plg::visit(cobj, v)), const Fn&>);
#ifndef _MSC_VER
			static_assert(std::is_same_v<decltype(plg::visit(std::move(obj), v)), Fn&&>);
			static_assert(std::is_same_v<decltype(plg::visit(std::move(cobj), v)), const Fn&&>);
#endif // _MSC_VER
		}
		{// test call operator forwarding - multi variant, multi arg
			using V = plg::variant<int, long, double>;
			using V2 = plg::variant<int*, std::string>;
			V v(42l);
			V2 v2("hello");
			static_assert(std::is_same_v<decltype(plg::visit(obj, v, v2)), Fn&>);
			static_assert(std::is_same_v<decltype(plg::visit(cobj, v, v2)), const Fn&>);
#ifndef _MSC_VER
			static_assert(std::is_same_v<decltype(plg::visit(std::move(obj), v, v2)), Fn&&>);
			static_assert(std::is_same_v<decltype(plg::visit(std::move(cobj), v, v2)), const Fn&&>);
#endif // _MSC_VER
		}
		{
			using V = plg::variant<int, long, double, std::string>;
			V v1(42l), v2("hello"), v3(101), v4(1.1);
			static_assert(std::is_same_v<decltype(plg::visit(obj, v1, v2, v3, v4)), Fn&>);
			static_assert(std::is_same_v<decltype(plg::visit(cobj, v1, v2, v3, v4)), const Fn&>);
#ifndef _MSC_VER
			static_assert(std::is_same_v<decltype(plg::visit(std::move(obj), v1, v2, v3, v4)), Fn&&>);
			static_assert(std::is_same_v<decltype(plg::visit(std::move(cobj), v1, v2, v3, v4)), const Fn&&>);
#endif // _MSC_VER
		}
		{
			using V = plg::variant<int, long, double, int*, std::string>;
			V v1(42l), v2("hello"), v3(nullptr), v4(1.1);
			static_assert(std::is_same_v<decltype(plg::visit(obj, v1, v2, v3, v4)), Fn&>);
			static_assert(std::is_same_v<decltype(plg::visit(cobj, v1, v2, v3, v4)), const Fn&>);
#ifndef _MSC_VER
			static_assert(std::is_same_v<decltype(plg::visit(std::move(obj), v1, v2, v3, v4)), Fn&&>);
			static_assert(std::is_same_v<decltype(plg::visit(std::move(cobj), v1, v2, v3, v4)), const Fn&&>);
#endif // _MSC_VER
		}
	}

	void test_constexpr() {
		constexpr ReturnFirst obj{};
		constexpr ReturnArity aobj{};
		{
			using V = plg::variant<int>;
			constexpr V v(42);
			static_assert(plg::visit(obj, v) == 42);
		}
		{
			using V = plg::variant<short, long, char>;
			constexpr V v(42l);
			static_assert(plg::visit(obj, v) == 42);
		}
		{
			using V1 = plg::variant<int>;
			using V2 = plg::variant<int, char*, long long>;
			using V3 = plg::variant<bool, int, int>;
			constexpr V1 v1;
			constexpr V2 v2(nullptr);
			constexpr V3 v3;
			static_assert(plg::visit(aobj, v1, v2, v3) == 3);
		}
		{
			using V1 = plg::variant<int>;
			using V2 = plg::variant<int, char*, long long>;
			using V3 = plg::variant<void*, int, int>;
			constexpr V1 v1;
			constexpr V2 v2(nullptr);
			constexpr V3 v3;
			static_assert(plg::visit(aobj, v1, v2, v3) == 3);
		}
		{
			using V = plg::variant<int, long, double, int*>;
			constexpr V v1(42l), v2(101), v3(nullptr), v4(1.1);
			static_assert(plg::visit(aobj, v1, v2, v3, v4) == 4);
		}
		{
			using V = plg::variant<int, long, double, long long, int*>;
			constexpr V v1(42l), v2(101), v3(nullptr), v4(1.1);
			static_assert(plg::visit(aobj, v1, v2, v3, v4) == 4);
		}
	}

	void test_exceptions() {
#if TEST_HAS_NO_EXCEPTIONS
		ReturnArity obj{};
		auto test = [&](auto&&... args) {
			try {
				plg::visit(obj, args...);
			} catch (const plg::bad_variant_access&) {
				return true;
			} catch (...) {
			}
			return false;
		};
		{
			puts("a");
			using V = plg::variant<int, MakeEmptyT>;
			V v;
			makeEmpty(v);
			REQUIRE(test(v));
		}
		{
			puts("b");
			using V = plg::variant<int, MakeEmptyT>;
			using V2 = plg::variant<long, std::string, void*>;
			V v;
			makeEmpty(v);
			V2 v2("hello");
			REQUIRE(test(v, v2));
		}
		{
			puts("c");
			using V = plg::variant<int, MakeEmptyT>;
			using V2 = plg::variant<long, std::string, void*>;
			V v;
			makeEmpty(v);
			V2 v2("hello");
			REQUIRE(test(v2, v));
		}
		{
			puts("d");
			using V = plg::variant<int, MakeEmptyT>;
			using V2 = plg::variant<long, std::string, void*, MakeEmptyT>;
			V v;
			makeEmpty(v);
			V2 v2;
			makeEmpty(v2);
			REQUIRE(test(v, v2));
		}
		{
			puts("e");
			using V = plg::variant<int, long, double, MakeEmptyT>;
			V v1(42l), v2(101), v3(202), v4(1.1);
			makeEmpty(v1);
			REQUIRE(test(v1, v2, v3, v4));
		}
		{
			puts("f");
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

	// See https://llvm.org/PR31916
	void test_caller_accepts_nonconst() {
		struct A {};
		struct Visitor {
			void operator()(A&) {}
		};
		plg::variant<A> v;
		plg::visit(Visitor{}, v);
	}
} // namespace

TEST_CASE("variant > visit > no return", "[variant]") {
	SECTION("void visit(Visitor&& vis, Variants&&... vars)") {
		test_argument_forwarding();
		test_return_type();
		test_constexpr();
		test_exceptions();
		test_caller_accepts_nonconst();
		test_call_operator_forwarding();
	}
}
