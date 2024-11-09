#include <catch_amalgamated.hpp>

#include <plugify/string.hpp>
#include <plugify/vector.hpp>

#include <chrono>
#include <plugify/compat_format.hpp>

using namespace std::literals;

static_assert(sizeof(plg::vector<uint32_t>) == 24);

using vint = plg::vector<int>;
using sint = plg::vector<std::string>;

#define LONGSTR "_________________________________________________"

namespace {
	template <typename T>
	void testConstruction() {
		T a;
		
		for (int i = 0; i < 100; ++i) {
			a.push_back(i + 100);
		}

		for (size_t i = 0; i < 100; ++i) {
			REQUIRE(static_cast<int>(i + 100) == a[i]);
		}

		const auto b = T(5, 10);
		REQUIRE(b.size() == 5);
		REQUIRE(b.back() == 10);

		const auto c = T({ 1, 2, 3 });
		REQUIRE(c.size() == 3);
		REQUIRE(c[0] == 1);
		REQUIRE(c[1] == 2);
		REQUIRE(c[2] == 3);
	}
	
	template <typename T>
	void testErase() {
		T a;

		REQUIRE(a.empty());

		for (int i = 0; i < 10; ++i) {
			a.push_back(i);
		}

		REQUIRE_FALSE(a.empty());

		a.erase(a.begin() + 5);

		REQUIRE(a.size() == 9);
		REQUIRE(a[4] == 4);
		REQUIRE(a[5] == 6);

		a.erase(a.begin(), a.begin() + 2);
		REQUIRE(a.size() == 7);
		REQUIRE(a[0] == 2);
	}

	template <typename T>
	void testSelfMove(typename T::value_type v) {
		T a;

		a.push_back(v);
		for (int i = 0; i < 20; ++i) {
			a.push_back(std::move(a.back()));
		}

		REQUIRE(a.front() == typename T::value_type());
		REQUIRE(a.back() == v);
	}

	template <typename T>
	void testMoveOnly(typename T::value_type v) {
		T a;

		a.push_back(std::move(v));
		for (int i = 0; i < 20; ++i) {
			a.push_back(std::move(a.back()));
		}

		REQUIRE(a.front() == typename T::value_type());
		REQUIRE(a.back() != typename T::value_type());
	}
	
	template <typename T>
	void testAssignment() {
		T a;

		for (int i = 0; i < 10; ++i) {
			a.push_back(i);
		}

		REQUIRE(a[5] == 5);

		T b;
		b = a;

		REQUIRE(b[5] == 5);

		for (int i = 0; i < 10; ++i) {
			REQUIRE(a[i] == b[i]);
		}
		
		T c;
		c = std::move(b);

		REQUIRE(b.empty());

		for (size_t i = 0; i < 10; ++i) {
			REQUIRE(a[i] == c[i]);
		}

		REQUIRE_FALSE(a == b);
		REQUIRE(a == c);

		std::swap(b, c);
		REQUIRE(a == b);
		REQUIRE_FALSE(a == c);
	}

	template <typename T>
	void testSBO([[maybe_unused]] size_t expectedSBOSize)
	{
#if PLUGIFY_VECTOR_SBO
		if (!T::sbo_enabled()) {
			return;
		}

		constexpr size_t maxSBO = static_cast<size_t>(T::sbo_max_objects());
		REQUIRE(maxSBO == expectedSBOSize);

		T a;

		for (size_t i = 0; i < maxSBO; ++i) {
			const auto val = i + 100;
			if constexpr (std::is_same_v<typename T::value_type, plg::string>) {
				a.push_back(plg::to_string(val));
			} else {
				a.push_back(typename T::value_type(val));
			}
		}
		REQUIRE(a.sbo_active());

		T b = a;
		REQUIRE(b.sbo_active());
		b.push_back({});
		REQUIRE_FALSE(b.sbo_active());
		b.pop_back();
		b.shrink_to_fit();
		REQUIRE(b.sbo_active());
#endif
	}
}

TEST_CASE("vector", "[vector]") {
	SECTION("Constructor, Empty") {
		vint vec;
		REQUIRE(0 == vec.size());
		REQUIRE(0 == vec.capacity());
	}

	SECTION("Constructor, CountAndValue") {
		size_t count = 100;
		int value = 5;
		vint vec(count, value); // { 5, 5 }

		for (size_t i = 0; i < count; i++) {
			REQUIRE(value == vec.at(i));
			REQUIRE(value == vec.at(i));
		}
	}

	SECTION("Constructor, WithIterators") {
		vint original({ 1, 2, 3, 4 });
		vint copy(original.begin(), original.end());

		REQUIRE(4 == copy.size());

		for (size_t i = 0; i < 4; i++) {
			REQUIRE(original.at(i) == copy.at(i));
		}
	}

	SECTION("Constructor, WithOtherVector") {
		vint original({ 1, 2, 3, 4 });
		vint copy(original);

		REQUIRE(4 == copy.size());

		for (size_t i = 0; i < 4; i++) {
			REQUIRE(original.at(i) == copy.at(i));
		}
	}

	SECTION("Constructor, Move") {
		vint original({ 1, 2, 3, 4 });
		vint copy = std::move(original);

		REQUIRE(4 == copy.size());
		REQUIRE(1 == copy.at(0));
		REQUIRE(2 == copy.at(1));
		REQUIRE(3 == copy.at(2));
		REQUIRE(4 == copy.at(3));

		REQUIRE(original.empty());
	}

	SECTION("Constructor, WithInitList") {
		std::initializer_list<int> list{ 1, 2, 3, 4 };
		vint vec(list);

		REQUIRE(list.size() == vec.size());

		for (size_t i = 0; i < list.size(); i++) {
			REQUIRE(*(list.begin() + i) == vec.at(i));
		}
	}

	SECTION("Destructor, FreeStack") {
		vint vec;
		vec.~vector();

		REQUIRE(nullptr == vec.data());
	}

	SECTION("Destructor, FreeHeap") {
		vint* vec = new vint();
		vec->push_back(1);
		delete vec;
	}

	SECTION("Operator, AssignToOtherVector") {
		vint original({ 1, 2, 3, 4 });
		vint copy;
		copy = original;
		REQUIRE(4 == copy.size());

		for (size_t i = 0; i < 4; i++) {
			REQUIRE(original.at(i) == copy.at(i));
		}
	}

	SECTION("Function, Assign") {
		size_t count = 3;
		int value = 5;
		vint vec({ 1, 2, 3, 4 });
		vec.assign(count, value);

		REQUIRE(count == vec.size());
		for (size_t i = 0; i < vec.size(); i++) {
			REQUIRE(value == vec.at(i));
		}
	}

	SECTION("Function, AssignWithIterator") {
		vint original({ 10, 20, 30, 40, 50 });
		vint vec({ 1, 2, 3, 4 });
		vec.assign(original.begin(), original.end());

		REQUIRE(5 == vec.size());
		REQUIRE(10 == vec.at(0));
		REQUIRE(20 == vec.at(1));
		REQUIRE(30 == vec.at(2));
		REQUIRE(40 == vec.at(3));
		REQUIRE(50 == vec.at(4));
	}

	SECTION("Access, At") {
		vint vec({ 1, 2, 3, 4 });

		REQUIRE(vec.at(0) == 1);
		REQUIRE(vec.at(1) == 2);
		REQUIRE(vec.at(2) == 3);
		REQUIRE(vec.at(3) == 4);

		REQUIRE_THROWS_AS(vec.at(10), std::out_of_range);
	}

	SECTION("Access, OperatorArray") {
		vint vec({ 1, 2, 3, 4 });

		REQUIRE(1 == vec[0]);
		REQUIRE(2 == vec[1]);
		REQUIRE(3 == vec[2]);
		REQUIRE(4 == vec[3]);
	}

	SECTION("Access, front") {
		vint vec({ 1, 2, 3, 4 });
		REQUIRE(1 == vec.front());
	}

	SECTION("Access, back") {
		vint vec({ 1, 2, 3, 4 });
		REQUIRE(4 == vec.back());
	}

	SECTION("Access, Data") {
		vint vec({ 1, 2, 3, 4 });
		int *data = vec.data();

		REQUIRE(1 == data[0]);
		REQUIRE(2 == data[1]);
		REQUIRE(3 == data[2]);
		REQUIRE(4 == data[3]);
	}

	SECTION("Iterators, Begin") {
		vint vec({ 1, 2, 3, 4 });
		REQUIRE(1 == *vec.begin());
	}

	SECTION("Iterators, End") {
		vint vec({ 1, 2, 3, 4 });
		REQUIRE(vec.begin() + vec.size() == vec.end());
	}

	SECTION("Iterators, Rbegin") {
		vint vec({ 1, 2, 3, 4 });
		REQUIRE(4 == *vec.rbegin());
	}

	SECTION("Iterators, Rend") {
		vint vec({ 1, 2, 3, 4 });
		REQUIRE(1 == *(vec.rend() - 1));
	}

	SECTION("Access, Back") {
		vint vec;
		vec.push_back(10);
		vec.push_back(20);
		REQUIRE(vec.back() == 20);
	}

	SECTION("Access, End") {
		vint vec;
		vec.push_back(10);
		vec.push_back(20);
		vint::iterator it = vec.end();
		REQUIRE(*(--it) == 20);
	}

	SECTION("Access, CEnd") {
		vint vec;
		vec.push_back(10);
		vec.push_back(20);
		vint::const_iterator it = vec.cend();
		REQUIRE(*(--it) == 20);
	}

	SECTION("Access, RBegin") {
		vint vec;
		vec.push_back(10);
		vec.push_back(20);
		vint::reverse_iterator it = vec.rbegin();
		REQUIRE(*it == 20);
	}

	SECTION("Access, CRBegin") {
		vint vec;
		vec.push_back(10);
		vec.push_back(20);
		vint::const_reverse_iterator it = vec.crbegin();
		REQUIRE(*it == 20);
	}

	SECTION("Access, GetAllocator") {
		vint vec;
		std::allocator<int> alloc = vec.get_allocator();
		int* p = alloc.allocate(1);  // allocate an int
		alloc.deallocate(p, 1);  // deallocate it
	}

	/*SECTION("Modifier, AppendRange") {
		vint vec;
		plg::vector<int> range = {10, 20, 30};
		vec.append_range(range.begin(), range.end());
		REQUIRE(vec[0], 10);
		REQUIRE(vec[1], 20);
		REQUIRE(vec[2] == 30);
	}*/

	SECTION("Modifier, Erase") {
		vint vec({ 1, 2, 3, 4 });
		vec.erase(vec.begin() + 1);

		REQUIRE(vec.size() == 3);
		REQUIRE(vec.at(0) == 1);
		REQUIRE(vec.at(1) == 3);
		REQUIRE(vec.at(2) == 4);
	}

	SECTION("Modifier, EraseWithRange") {
		{

			vint vec({ 1, 2, 3, 4 });
			vec.erase(vec.begin() + 1, vec.begin() + 3);

			REQUIRE(vec.size() == 2);
			REQUIRE(vec.at(0) == 1);
			REQUIRE(vec.at(1) == 4);
		}
		{
			sint vec({ "1" LONGSTR, "2" LONGSTR, "3" LONGSTR, "4" LONGSTR });
			vec.erase(vec.begin() + 1, vec.begin() + 3);

			REQUIRE(vec.size() == 2);
			REQUIRE(vec.at(0) == "1" LONGSTR);
			REQUIRE(vec.at(1) == "4" LONGSTR);
			vec.erase(vec.begin());
			REQUIRE(vec.at(0) == "4" LONGSTR);
			REQUIRE(vec.size() == 1);
		}
	}

	SECTION("Modifier, EraseIf") {
		vint vec;
		vec.push_back(10);
		vec.push_back(20);
		auto it = std::find_if(vec.begin(), vec.end(), [](int x){return x == 10;});
		if (it != vec.end()) {
			vec.erase(it);
		}
		REQUIRE(vec[0] == 20);
	}

	SECTION("Modifier, NonMemberErase") {
		vint vec;
		vec.push_back(10);
		vec.push_back(20);
		auto it = std::find(vec.begin(), vec.end(), 10);
		if (it != vec.end()) {
			vec.erase(it);
		}
		REQUIRE(vec[0] == 20);
	}

	SECTION("Modifier, PushBack") {
		vint vec;
		vec.push_back(9);

		REQUIRE(9 == vec.at(0));
	}

	SECTION("Modifier, PushBackMove") {
		vint vec;
		int value = 5;
		vec.push_back(std::move(value));

		REQUIRE(5 == vec.at(0));
	}

	SECTION("Modifier, EmplaceBack") {
		vint vec;
		vec.emplace_back(10);

		REQUIRE(vec[0] == 10);
	}

	SECTION("Modifier, EmplaceBackMove") {
		struct T {
			int a;
			double b;
			plg::string c;

			T(int _a, double _b, std::string &&_c) : a(_a) , b(_b), c(std::move(_c)) {}
		};

		plg::vector<T> objects;
		objects.emplace_back(42, 3.14, "foo");

		REQUIRE(1 == objects.size());
		REQUIRE(42 == objects.at(0).a);
		REQUIRE(3.14 == objects.at(0).b);
		REQUIRE("foo" == objects.at(0).c);
	}

	SECTION("Capacity, Empty") {
		vint vec;
		REQUIRE(vec.empty());

		vec.push_back(1);
		REQUIRE_FALSE(vec.empty());
	}

	SECTION("Capacity, Size") {
		vint vec({ 1, 2, 3, 4 });
		REQUIRE(4 == vec.size());
	}

	SECTION("Capacity, MaxSize") {
		vint vec;
		REQUIRE(vec.max_size() == 0x3fffffffffffffff);
	}

	SECTION("Capacity, Reserve") {
		vint vec;
		vec.reserve(100);

		REQUIRE(100 == vec.capacity());
	}

	SECTION("Capacity, ShrinkToFit") {
		vint vec({ 1, 2, 3, 4 });
		vec.reserve(100); // TODO change to resize
		vec.shrink_to_fit();
		REQUIRE(4 == vec.capacity());
	}

	SECTION("Modifier, Clear") {
		vint vec({ 1, 2, 3, 4 });
		vec.clear();

		REQUIRE(0 == vec.size());
	}

	SECTION("Modifier, InsertWithValue") {
		int value = 5;
		vint vec({ 1, 2, 3, 4 });
		auto it = vec.insert(vec.begin(), value);

		REQUIRE(5 == vec.size());
		REQUIRE(5 == *it);
		REQUIRE(5 == vec.at(0));
		for (size_t i = 1; i < 5; i++) {
			REQUIRE(static_cast<int>(i) == vec.at(i));
		}

		it = vec.insert(vec.end(), value);
		REQUIRE(6 == vec.size());
		REQUIRE(5 == *it);
		REQUIRE(5 == vec.at(5));
	}

	SECTION("Modifier, InsertWithValueMove") {
		int value = 5;
		vint vec({ 1, 2, 3, 4 });
		auto it = vec.insert(vec.begin(), std::move(value));

		REQUIRE(5 == vec.size());
		REQUIRE(5 == *it);
		REQUIRE(5 == vec.at(0));
		for (size_t i = 1; i < 5; i++) {
			REQUIRE(static_cast<int>(i) == vec.at(i));
		}
	}

	SECTION("Modifier, InsertWithCount") {
		{
			vint vec({ 1, 2, 3, 4, 5, 6 });
			auto it = vec.insert(vec.begin(), vec.begin() + 3, vec.begin() + 5);

			REQUIRE(8 == vec.size());
			REQUIRE(4 == *it);

			REQUIRE(4 == vec.at(0));
			REQUIRE(5 == vec.at(1));
			REQUIRE(1 == vec.at(2));

			REQUIRE(2 == vec.at(3));
			REQUIRE(3 == vec.at(4));
			REQUIRE(4 == vec.at(5));
			REQUIRE(5 == vec.at(6));
		}
		{
			vint vec({ 1, 2, 3, 4, 5, 6 });
			auto it = vec.insert(vec.begin(), vec.begin(), vec.end());

			REQUIRE(12 == vec.size());
			REQUIRE(1 == *it);

			REQUIRE(1 == vec.at(0));
			REQUIRE(2 == vec.at(1));
			REQUIRE(3 == vec.at(2));
			REQUIRE(4 == vec.at(3));
			REQUIRE(5 == vec.at(4));
			REQUIRE(6 == vec.at(5));

			REQUIRE(1 == vec.at(6));
			REQUIRE(2 == vec.at(7));
			REQUIRE(3 == vec.at(8));
			REQUIRE(4 == vec.at(9));
			REQUIRE(5 == vec.at(10));
			REQUIRE(6 == vec.at(11));
		}
		{
			sint vec({ "1" LONGSTR, "2" LONGSTR, "3" LONGSTR, "4" LONGSTR, "5" LONGSTR, "6" LONGSTR });
			vec.shrink_to_fit();
			auto it = vec.insert(vec.begin(), vec.begin(), vec.end());

			REQUIRE(12 == vec.size());
			REQUIRE("1" LONGSTR == *it);

			REQUIRE("1" LONGSTR == vec.at(0));
			REQUIRE("2" LONGSTR == vec.at(1));
			REQUIRE("3" LONGSTR == vec.at(2));
			REQUIRE("4" LONGSTR == vec.at(3));
			REQUIRE("5" LONGSTR == vec.at(4));
			REQUIRE("6" LONGSTR == vec.at(5));

			REQUIRE("1" LONGSTR == vec.at(6));
			REQUIRE("2" LONGSTR == vec.at(7));
			REQUIRE("3" LONGSTR == vec.at(8));
			REQUIRE("4" LONGSTR == vec.at(9));
			REQUIRE("5" LONGSTR == vec.at(10));
			REQUIRE("6" LONGSTR == vec.at(11));
		}
	}

	SECTION("Modifier, InsertWithIterators") {
		vint vec({ 1, 2, 3, 4 });
		vint other({ 10, 20, 30, 40 });

		int results[6] = { 1, 2, 20, 30, 3, 4 };

		auto it = vec.insert(vec.begin() + 2, other.begin() + 1, other.begin() + 3);
		REQUIRE(6 == vec.size());
		REQUIRE(20 == *it);

		for (size_t i = 0; i < 6; i++) {
			REQUIRE(vec.at(i) == results[i]);
		}
	}

	SECTION("Modifier, InsertWithInitList") {
		vint vec({ 1, 2, 3, 4 });
		std::initializer_list<int> list{ 20, 30 };

		int results[6] = { 1, 2, 20, 30, 3, 4 };

		auto it = vec.insert(vec.begin() + 2, list);
		REQUIRE(6 == vec.size());
		REQUIRE(20 == *it);

		for (size_t i = 0; i < 6; i++) {
			REQUIRE(vec.at(i) == results[i]);
		}
	}

	SECTION("Modifier, Emplace") {
		struct T {
			int a;
			double b;
			plg::string c;

			T(int _a, double _b, std::string &&_c) : a(_a) , b(_b), c(std::move(_c)) {}
		};

		plg::vector<T> objects;
		objects.emplace(objects.begin(), 42, 3.14, "foo");

		REQUIRE(1 == objects.size());
		REQUIRE(42 == objects.at(0).a);
		REQUIRE(3.14 == objects.at(0).b);
		REQUIRE("foo" == objects.at(0).c);
	}

	SECTION("Modifier, PopBack") {
		vint vec({ 1, 2, 3, 4 });
		int result[3] = { 1, 2, 3 };
		vec.pop_back();

		REQUIRE(3 == vec.size());

		for (size_t i = 0; i < 3; i++) {
			REQUIRE(result[i] == vec.at(i));
		}
	}

	SECTION("Modifier, Resize") {
		vint vec({ 1, 2, 3, 4 });
		vec.resize(10);

		REQUIRE(10 == vec.size());
		REQUIRE(1 == vec.at(0));
		REQUIRE(2 == vec.at(1));
		REQUIRE(3 == vec.at(2));
		REQUIRE(4 == vec.at(3));
		for (size_t i = 5; i < 10; i++) {
			REQUIRE(0 == vec.at(i));
		}

		vec.resize(2);
		REQUIRE(2 == vec.size());
		REQUIRE(1 == vec.at(0));
		REQUIRE(2 == vec.at(1));
	}

	SECTION("Modifier, ResizeWithValue") {
		vint vec({ 1, 2, 3, 4 });
		vec.resize(10, 999);

		REQUIRE(10 == vec.size());
		REQUIRE(1 == vec.at(0));
		REQUIRE(2 == vec.at(1));
		REQUIRE(3 == vec.at(2));
		REQUIRE(4 == vec.at(3));
		for (size_t i = 5; i < 10; i++) {
			REQUIRE(999 == vec.at(i));
		}

		vec.resize(2);
		REQUIRE(2 == vec.size());
		REQUIRE(1 == vec.at(0));
		REQUIRE(2 == vec.at(1));
	}

	SECTION("Modifier, Swap") {
		vint vec({ 1, 2, 3, 4 });
		vint other({ 10, 20, 30, 40, 50 });
		vec.swap(other);

		REQUIRE(5 == vec.size());
		REQUIRE(4 == other.size());

		for (size_t i = 0; i < 5; i++) {
			REQUIRE((i + 1) * 10 == static_cast<size_t>(vec.at(i)));
		}

		for (size_t i = 0; i < 4; i++) {
			REQUIRE(i + 1 == static_cast<size_t>(other.at(i)));
		}
	}

	SECTION("Modifier, StdSwap") {
		vint vec1;
		vec1.push_back(10);
		vint vec2;
		vec2.push_back(20);
		std::swap(vec1, vec2);
		REQUIRE(vec1[0] == 20);
		REQUIRE(vec2[0] == 10);
	}

	SECTION("Operator, Equal") {
		vint vec({ 1, 2, 3, 4 });
		vint other({ 1, 2, 3, 4 });
		vint different({ 1, 2, 3, 5 });

		REQUIRE(vec == other);
		REQUIRE_FALSE(vec == different);
	}

	SECTION("Operator, NotEqual") {
		vint vec({ 1, 2, 3, 4 });
		vint other({ 1, 2, 3, 4 });
		vint different({ 1, 2, 3, 5 });

		REQUIRE_FALSE(vec != other);
		REQUIRE(vec != different);
	}

	SECTION("Operator, Less") {
		vint vec({ 1, 2, 3, 4 });
		vint other({ 1, 2, 3, 4 });
		vint different({ 1, 3, 3, 4 });

		REQUIRE_FALSE(vec < other);
		REQUIRE(vec < different);
	}

	SECTION("Operator, More") {
		vint vec({ 1, 2, 3, 4 });
		vint other({ 1, 2, 3, 4 });
		vint different({ 1, 3, 3, 4 });

		REQUIRE_FALSE(vec > other);
		REQUIRE_FALSE(vec > different);
	}

	SECTION("Operator, LessOrEqual") {
		vint vec({ 1, 2, 3, 4 });
		vint other({ 1, 2, 3, 4 });
		vint different({ 1, 3, 3, 4 });

		REQUIRE(vec <= other);
		REQUIRE(vec <= different);
	}

	SECTION("Operator, MoreOrEqual") {
		vint vec({ 1, 2, 3, 4 });
		vint other({ 1, 2, 3, 4 });
		vint different({ 1, 3, 3, 4 });

		REQUIRE(vec >= other);
		REQUIRE_FALSE(vec >= different);
	}
	
	SECTION("Test, Vector")
	{
		testConstruction<plg::vector<int>>();
	}
	
	SECTION("Test, Vector")
	{
		testErase<plg::vector<int>>();
	}

	SECTION("Test, SelfMove")
	{
		testSelfMove<plg::vector<plg::string>>("hellohellohellohellohellohellohellohellohellohellohellohellohello");
		testMoveOnly<plg::vector<std::unique_ptr<int>>>(std::make_unique<int>(42));
	}

	SECTION("Test, Assignment")
	{
		testAssignment<plg::vector<int>>();
	}

	SECTION("Test, Construction")
	{
		testConstruction<plg::vector<int>>();
	}

	SECTION("Test, Erase")
	{
		testErase<plg::vector<int>>();
	}

	SECTION("Test, SelfMove")
	{
		testSelfMove<plg::vector<plg::string>>("hello");
		testMoveOnly<plg::vector<std::unique_ptr<int>>>(std::make_unique<int>(42));
	}

	SECTION("Test, Assignment")
	{
		testAssignment<plg::vector<int>>();
	}

	SECTION("Test, SBO")
	{
		testSBO<plg::vector<char>>(23);
		testSBO<plg::vector<int>>(5);
		testSBO<plg::vector<size_t>>(2);
		testSBO<plg::vector<plg::string>>(0);
	}
}
