#include <catch_amalgamated.hpp>

#include <app/counter.hpp>
#include <app/vec_tester.hpp>
#include <plugify/vector.hpp>

#include <plugify/compat_format.hpp>

#include <forward_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <iostream>
#include <ranges>
#include <iterator>

template <class T>
class FooInputIterator {
	T* _ptr{};
	
public:
	using iterator_category = std::input_iterator_tag;
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using reference = T&;

	explicit FooInputIterator(T* ptr)
		: _ptr(ptr) {}

	FooInputIterator(const FooInputIterator& other)
		: _ptr(other._ptr) {}

	FooInputIterator(FooInputIterator&& other) noexcept
		: _ptr(other._ptr) {}

	operator const T*() const noexcept {
		return _ptr;
	}

	auto operator*() const -> T& {
		return *_ptr;
	}
	auto operator->() const -> T* {
		return _ptr;
	}
	auto operator++() -> FooInputIterator& {
		++_ptr;
		return *this;
	}
	auto operator++(int) -> FooInputIterator {
		auto tmp = *this;
		++*this;
		return tmp;
	}
	auto operator--() -> FooInputIterator& {
		--_ptr;
		return *this;
	}
	auto operator--(int) -> FooInputIterator {
		auto tmp = *this;
		--*this;
		return tmp;
	}

	operator difference_type() const noexcept {
		return reinterpret_cast<difference_type>(_ptr);
	}

	friend auto operator!=(FooInputIterator const& a, FooInputIterator const& b) {
		return a._ptr != b._ptr;
	}
	friend auto operator==(FooInputIterator const& a, FooInputIterator const& b) {
		return a._ptr == b._ptr;
	}
	friend auto operator-(FooInputIterator const& a, FooInputIterator const& b) {
		return FooInputIterator{reinterpret_cast<T*>(a._ptr - b._ptr)};
	}
};

TEST_CASE("vector operation > insert", "[vector]") {
	
	SECTION("insert_single") {
		// extremely similar code to "emplace"
		
		for (size_t s = 0; s < 6; ++s) {
			auto vec_a = plg::vector<std::string>();
			auto vec_b = plg::vector<std::string>();
			for (size_t ins = 0; ins < s; ++ins) {
				vec_a.emplace_back(std::to_string(ins));
				vec_b.emplace_back(std::to_string(ins));
			}

			for (size_t i = 0; i <= vec_a.size(); ++i) {
				auto va = vec_a;
				auto vb = vec_b;

				// moved
				auto ita = va.insert(va.cbegin() + std::ptrdiff_t(i), "999");
				auto itb = vb.insert(vb.cbegin() + std::ptrdiff_t(i), "999");
				REQUIRE(*ita == *itb);
				*ita += 'x';
				*itb += 'x';

				// constexpr => copied
				std::string x = "asdf";
				ita = va.insert(va.cbegin() + i, x);
				itb = vb.insert(vb.cbegin() + std::ptrdiff_t(i), x);
				REQUIRE(*ita == x);
				*ita += 'g';
				*itb += 'g';

				assert_eq(va, vb);
			}
		}
	}

	// iterator insert( const_iterator pos, size_type count, const T& value );
	SECTION("insert_copies") {
		Counter counts;
		INFO(counts);

		for (size_t s = 0; s < 6; ++s) {
			auto vec_a = plg::vector<Counter::Obj>();
			auto vec_b = plg::vector<Counter::Obj>();
			for (size_t ins = 0; ins < s; ++ins) {
				vec_a.emplace_back(ins, counts);
				vec_b.emplace_back(ins, counts);
			}

			for (size_t i = 0; i <= vec_a.size(); ++i) {
				auto va = vec_a;
				auto vb = vec_b;

				// moved
				auto c = Counter::Obj(999, counts);
				auto ita = va.insert(va.cbegin() + i, 7, c);
				auto itb = vb.insert(vb.cbegin() + std::ptrdiff_t(i), 7, c);
				REQUIRE(*ita == *itb);
				assert_eq(va, vb);
			}
		}
	}

	// template< class InputIt > iterator insert(const_iterator pos, InputIt first, InputIt last);
	SECTION("insert_input_iterator") {
		Counter counts;
		INFO(counts);

		auto va = plg::vector<Counter::Obj>();
		auto vb = plg::vector<Counter::Obj>();
		va.emplace_back(1, counts);
		vb.emplace_back(1, counts);
		va.emplace_back(2, counts);
		vb.emplace_back(2, counts);
		va.emplace_back(3, counts);
		vb.emplace_back(3, counts);
		assert_eq(va, vb);

		auto data = std::array<Counter::Obj, 5>{{
				Counter::Obj(10, counts),
				Counter::Obj(11, counts),
				Counter::Obj(12, counts),
				Counter::Obj(13, counts),
				Counter::Obj(14, counts),
		}};

		auto it_begin = FooInputIterator<Counter::Obj>(data.data());
		auto it_end = FooInputIterator<Counter::Obj>(data.data() + data.size());

		counts("before insert");
		assert_eq(va, vb);
		va.insert(va.begin() + 1, it_begin, it_end);
		vb.insert(vb.begin() + 1, it_begin, it_end);
		counts("after insert");
		assert_eq(va, vb);

		va.insert(va.end(), it_begin, it_end);
		vb.insert(vb.end(), it_begin, it_end);
		assert_eq(va, vb);

		auto ita = va.insert(va.begin() + std::ptrdiff_t(va.size()) / 2, data.begin(), data.end());
		auto itb = vb.insert(vb.begin() + vb.size() / 2, data.begin(), data.end());
		REQUIRE(*ita == *itb);
		assert_eq(va, vb);

		va.insert(va.end(), data.begin(), data.end());
		vb.insert(vb.end(), data.begin(), data.end());
		assert_eq(va, vb);
		va.insert(va.begin(), data.begin(), data.end());
		vb.insert(vb.begin(), data.begin(), data.end());
		assert_eq(va, vb);

		va.insert(va.begin() + 3, data.begin(), data.begin());
		vb.insert(vb.begin() + 3, data.begin(), data.begin());
		assert_eq(va, vb);

		va.insert(va.begin() + 3, it_begin, it_begin);
		vb.insert(vb.begin() + 3, it_begin, it_begin);
		assert_eq(va, vb);
	}

	// iterator insert(const_iterator pos, std::initializer_list<T> ilist);
}
