#include <catch_amalgamated.hpp>

#include <plg/vector.hpp>

#include <ostream>
#include <string_view>

using namespace std::literals;

TEST_CASE("vector iterator > rbegin_rend", "[vector]") {

	SECTION("rbegin") {
		plg::vector<int> v;
		const auto& c = v;

		REQUIRE(v.rbegin().base() == v.end());

		REQUIRE(v.rbegin() == v.rbegin());
		REQUIRE(v.rbegin() == v.crbegin());
		REQUIRE(v.rbegin() == v.rend());
		REQUIRE(v.rbegin() == v.crend());

		REQUIRE(v.rbegin() == c.rbegin());
		REQUIRE(v.rbegin() == c.crbegin());
		REQUIRE(v.rbegin() == c.rend());
		REQUIRE(v.rbegin() == c.crend());

		REQUIRE(c.rbegin() == v.rbegin());
		REQUIRE(c.rbegin() == v.crbegin());
		REQUIRE(c.rbegin() == v.rend());
		REQUIRE(c.rbegin() == v.crend());

		REQUIRE(c.rbegin() == c.rbegin());
		REQUIRE(c.rbegin() == c.crbegin());
		REQUIRE(c.rbegin() == c.rend());
		REQUIRE(c.rbegin() == c.crend());

		v.push_back(1);

		REQUIRE(1 == *v.rbegin());
		REQUIRE(1 == *c.rbegin());

		REQUIRE(v.rbegin().base() == v.end());

		REQUIRE(v.rbegin() == v.rbegin());
		REQUIRE(v.rbegin() == v.crbegin());
		REQUIRE(!(v.rbegin() == v.rend()));
		REQUIRE(!(v.rbegin() == v.crend()));

		REQUIRE(std::next(v.rbegin()) == v.rend());
		REQUIRE(std::next(v.rbegin()) == v.crend());

		REQUIRE(v.rbegin() == c.rbegin());
		REQUIRE(v.rbegin() == c.crbegin());
		REQUIRE(!(v.rbegin() == c.rend()));
		REQUIRE(!(v.rbegin() == c.crend()));

		REQUIRE(std::next(v.rbegin()) == c.rend());
		REQUIRE(std::next(v.rbegin()) == c.crend());

		REQUIRE(c.rbegin() == v.rbegin());
		REQUIRE(c.rbegin() == v.crbegin());
		REQUIRE(!(c.rbegin() == v.rend()));
		REQUIRE(!(c.rbegin() == v.crend()));

		REQUIRE(std::next(c.rbegin()) == v.rend());
		REQUIRE(std::next(c.rbegin()) == v.crend());

		REQUIRE(c.rbegin() == c.rbegin());
		REQUIRE(c.rbegin() == c.crbegin());
		REQUIRE(!(c.rbegin() == c.rend()));
		REQUIRE(!(c.rbegin() == c.crend()));

		REQUIRE(std::next(c.rbegin()) == c.rend());
		REQUIRE(std::next(c.rbegin()) == c.crend());

		v.push_back(2);

		REQUIRE(2 == *v.rbegin());
		REQUIRE(2 == *c.rbegin());

		REQUIRE(v.rbegin().base() == v.end());

		REQUIRE(v.rbegin() == v.rbegin());
		REQUIRE(v.rbegin() == v.crbegin());
		REQUIRE(!(v.rbegin() == v.rend()));
		REQUIRE(!(v.rbegin() == v.crend()));

		REQUIRE(std::next(v.rbegin(), 2) == v.rend());
		REQUIRE(std::next(v.rbegin(), 2) == v.crend());

		REQUIRE(v.rbegin() == c.rbegin());
		REQUIRE(v.rbegin() == c.crbegin());
		REQUIRE(!(v.rbegin() == c.rend()));
		REQUIRE(!(v.rbegin() == c.crend()));

		REQUIRE(std::next(v.rbegin(), 2) == c.rend());
		REQUIRE(std::next(v.rbegin(), 2) == c.crend());

		REQUIRE(c.rbegin() == v.rbegin());
		REQUIRE(c.rbegin() == v.crbegin());
		REQUIRE(!(c.rbegin() == v.rend()));
		REQUIRE(!(c.rbegin() == v.crend()));

		REQUIRE(std::next(c.rbegin(), 2) == v.rend());
		REQUIRE(std::next(c.rbegin(), 2) == v.crend());

		REQUIRE(c.rbegin() == c.rbegin());
		REQUIRE(c.rbegin() == c.crbegin());
		REQUIRE(!(c.rbegin() == c.rend()));
		REQUIRE(!(c.rbegin() == c.crend()));

		REQUIRE(std::next(c.rbegin(), 2) == c.rend());
		REQUIRE(std::next(c.rbegin(), 2) == c.crend());

		v.push_back(3);

		REQUIRE(3 == *v.rbegin());
		REQUIRE(3 == *c.rbegin());

		REQUIRE(v.rbegin().base() == v.end());

		REQUIRE(v.rbegin() == v.rbegin());
		REQUIRE(v.rbegin() == v.crbegin());
		REQUIRE(!(v.rbegin() == v.rend()));
		REQUIRE(!(v.rbegin() == v.crend()));

		REQUIRE(v.rbegin() == c.rbegin());
		REQUIRE(v.rbegin() == c.crbegin());
		REQUIRE(!(v.rbegin() == c.rend()));
		REQUIRE(!(v.rbegin() == c.crend()));

		REQUIRE(c.rbegin() == v.rbegin());
		REQUIRE(c.rbegin() == v.crbegin());
		REQUIRE(!(c.rbegin() == v.rend()));
		REQUIRE(!(c.rbegin() == v.crend()));

		REQUIRE(c.rbegin() == c.rbegin());
		REQUIRE(c.rbegin() == c.crbegin());
		REQUIRE(!(c.rbegin() == c.rend()));
		REQUIRE(!(c.rbegin() == c.crend()));

		v.clear();

		REQUIRE(v.rbegin().base() == v.end());

		REQUIRE(v.rbegin() == v.rbegin());
		REQUIRE(v.rbegin() == v.crbegin());
		REQUIRE(v.rbegin() == v.rend());
		REQUIRE(v.rbegin() == v.crend());

		REQUIRE(v.rbegin() == c.rbegin());
		REQUIRE(v.rbegin() == c.crbegin());
		REQUIRE(v.rbegin() == c.rend());
		REQUIRE(v.rbegin() == c.crend());

		REQUIRE(c.rbegin() == v.rbegin());
		REQUIRE(c.rbegin() == v.crbegin());
		REQUIRE(c.rbegin() == v.rend());
		REQUIRE(c.rbegin() == v.crend());

		REQUIRE(c.rbegin() == c.rbegin());
		REQUIRE(c.rbegin() == c.crbegin());
		REQUIRE(c.rbegin() == c.rend());
		REQUIRE(c.rbegin() == c.crend());
	}

	SECTION("crbegin") {
		plg::vector<int> v;

		REQUIRE(v.crbegin() == v.rbegin());

		v.push_back(1);

		REQUIRE(1 == *v.crbegin());
		REQUIRE(v.crbegin() == v.rbegin());

		v.push_back(2);

		REQUIRE(2 == *v.crbegin());
		REQUIRE(v.crbegin() == v.rbegin());

		v.push_back(3);

		REQUIRE(3 == *v.crbegin());
		REQUIRE(v.crbegin() == v.rbegin());
	}

	SECTION("rend") {
		plg::vector<int> v;
		const auto& c = v;
		
		REQUIRE(v.rend().base() == v.begin());

		REQUIRE(v.rend() == v.rend());
		REQUIRE(v.rend() == v.crend());
		REQUIRE(v.rend() == v.rbegin());
		REQUIRE(v.rend() == v.crbegin());

		REQUIRE(v.rend() == c.rend());
		REQUIRE(v.rend() == c.crend());
		REQUIRE(v.rend() == c.rbegin());
		REQUIRE(v.rend() == c.crbegin());

		REQUIRE(c.rend() == v.rend());
		REQUIRE(c.rend() == v.crend());
		REQUIRE(c.rend() == v.rbegin());
		REQUIRE(c.rend() == v.crbegin());

		REQUIRE(c.rend() == c.rend());
		REQUIRE(c.rend() == c.crend());
		REQUIRE(c.rend() == c.rbegin());
		REQUIRE(c.rend() == c.crbegin());

		v.push_back(1);

		REQUIRE(1 == *std::prev(v.rend()));
		REQUIRE(1 == *std::prev(c.rend()));

		REQUIRE(v.rend().base() == v.begin());

		REQUIRE(v.rend() == v.rend());
		REQUIRE(v.rend() == v.crend());
		REQUIRE(!(v.rend() == v.rbegin()));
		REQUIRE(!(v.rend() == v.crbegin()));

		REQUIRE(std::prev(v.rend()) == v.rbegin());
		REQUIRE(std::prev(v.rend()) == v.crbegin());

		REQUIRE(v.rend() == c.rend());
		REQUIRE(v.rend() == c.crend());
		REQUIRE(!(v.rend() == c.rbegin()));
		REQUIRE(!(v.rend() == c.crbegin()));

		REQUIRE(std::prev(v.rend()) == c.rbegin());
		REQUIRE(std::prev(v.rend()) == c.crbegin());

		REQUIRE(c.rend() == v.rend());
		REQUIRE(c.rend() == v.crend());
		REQUIRE(!(c.rend() == v.rbegin()));
		REQUIRE(!(c.rend() == v.crbegin()));

		REQUIRE(std::prev(c.rend()) == v.rbegin());
		REQUIRE(std::prev(c.rend()) == v.crbegin());

		REQUIRE(c.rend() == c.rend());
		REQUIRE(c.rend() == c.crend());
		REQUIRE(!(c.rend() == c.rbegin()));
		REQUIRE(!(c.rend() == c.crbegin()));

		REQUIRE(std::prev(c.rend()) == c.rbegin());
		REQUIRE(std::prev(c.rend()) == c.crbegin());

		v.push_back(2);

		REQUIRE(1 == *std::prev(v.rend()));
		REQUIRE(1 == *std::prev(c.rend()));

		REQUIRE(v.rend().base() == v.begin());

		REQUIRE(v.rend() == v.rend());
		REQUIRE(v.rend() == v.crend());
		REQUIRE(!(v.rend() == v.rbegin()));
		REQUIRE(!(v.rend() == v.crbegin()));

		REQUIRE(std::prev(v.rend(), 2) == v.rbegin());
		REQUIRE(std::prev(v.rend(), 2) == v.crbegin());

		REQUIRE(v.rend() == c.rend());
		REQUIRE(v.rend() == c.crend());
		REQUIRE(!(v.rend() == c.rbegin()));
		REQUIRE(!(v.rend() == c.crbegin()));

		REQUIRE(std::prev(v.rend(), 2) == c.rbegin());
		REQUIRE(std::prev(v.rend(), 2) == c.crbegin());

		REQUIRE(c.rend() == v.rend());
		REQUIRE(c.rend() == v.crend());
		REQUIRE(!(c.rend() == v.rbegin()));
		REQUIRE(!(c.rend() == v.crbegin()));

		REQUIRE(std::prev(c.rend(), 2) == v.rbegin());
		REQUIRE(std::prev(c.rend(), 2) == v.crbegin());

		REQUIRE(c.rend() == c.rend());
		REQUIRE(c.rend() == c.crend());
		REQUIRE(!(c.rend() == c.rbegin()));
		REQUIRE(!(c.rend() == c.crbegin()));

		REQUIRE(std::prev(c.rend(), 2) == c.rbegin());
		REQUIRE(std::prev(c.rend(), 2) == c.crbegin());

		v.insert(v.begin(), 3);

		REQUIRE(3 == *std::prev(v.rend()));
		REQUIRE(3 == *std::prev(c.rend()));

		REQUIRE(v.rend().base() == v.begin());

		REQUIRE(v.rend() == v.rend());
		REQUIRE(v.rend() == v.crend());
		REQUIRE(!(v.rend() == v.rbegin()));
		REQUIRE(!(v.rend() == v.crbegin()));

		REQUIRE(v.rend() == c.rend());
		REQUIRE(v.rend() == c.crend());
		REQUIRE(!(v.rend() == c.rbegin()));
		REQUIRE(!(v.rend() == c.crbegin()));

		REQUIRE(c.rend() == v.rend());
		REQUIRE(c.rend() == v.crend());
		REQUIRE(!(c.rend() == v.rbegin()));
		REQUIRE(!(c.rend() == v.crbegin()));

		REQUIRE(c.rend() == c.rend());
		REQUIRE(c.rend() == c.crend());
		REQUIRE(!(c.rend() == c.rbegin()));
		REQUIRE(!(c.rend() == c.crbegin()));

		v.clear();

		REQUIRE(v.rend() == v.rend());
		REQUIRE(v.rend() == v.crend());
		REQUIRE(v.rend() == v.rbegin());
		REQUIRE(v.rend() == v.crbegin());

		REQUIRE(v.rend() == c.rend());
		REQUIRE(v.rend() == c.crend());
		REQUIRE(v.rend() == c.rbegin());
		REQUIRE(v.rend() == c.crbegin());

		REQUIRE(c.rend() == v.rend());
		REQUIRE(c.rend() == v.crend());
		REQUIRE(c.rend() == v.rbegin());
		REQUIRE(c.rend() == v.crbegin());

		REQUIRE(c.rend() == c.rend());
		REQUIRE(c.rend() == c.crend());
		REQUIRE(c.rend() == c.rbegin());
		REQUIRE(c.rend() == c.crbegin());
	}

	SECTION("crend") {
		plg::vector<int> v;
		
		REQUIRE(v.crend() == v.rend());

		v.push_back(1);

		REQUIRE(1 == *std::prev(v.crend()));
		REQUIRE(v.crend() == v.rend());

		v.push_back(2);

		REQUIRE(1 == *std::prev(v.crend()));
		REQUIRE(v.crend() == v.rend());

		v.insert(v.begin(), 3);

		REQUIRE(3 == *std::prev(v.crend()));
		REQUIRE(v.crend() == v.rend());
	}

	SECTION("rbegin_rend") {
		auto a = plg::vector<char>();
		for(auto c: "hello world!"sv) {
			a.push_back(c);
		}
		auto it = a.rbegin();
		auto itc = a.crbegin();
		REQUIRE(*it == '!');
		REQUIRE(*itc == '!');
		++it;
		REQUIRE(*it == 'd');
		REQUIRE(*itc == '!');
		++itc;
		REQUIRE(*itc == 'd');
		*it = 'x';
		REQUIRE(*it == 'x');
		REQUIRE(*itc == 'x');

		auto const& ca = a;
		auto itc2 = ca.rbegin();
		auto v2 = plg::vector<char>();
		while(itc2 != ca.rend()) {
			v2.push_back(*itc2);
			++itc2;
		}
		REQUIRE(std::string_view(v2.data(), v2.size()) == "!xlrow olleh");
	}
}
