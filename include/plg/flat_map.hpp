#pragma once

#include "plg/macro.hpp"

#ifdef __cpp_lib_flat_map
#include <flat_map>
namespace plg {
	template<typename K, typename V, typename C = std::less<K>>
	using flat_map = std::flat_map<K, V, C>;
}
#else
#include "plg/vector.hpp"

namespace plg {
	namespace detail {
		template < typename T, typename U, typename = void >
		struct is_transparent
			: std::false_type {};

		template < typename T, typename U >
		struct is_transparent<T, U, std::void_t<typename T::is_transparent>>
			: std::true_type {};

		template < typename T, typename U >
		inline constexpr bool is_transparent_v = is_transparent<T, U>::value;

		template<typename Iter, typename Compare>
		constexpr bool is_sorted(Iter first, Iter last, Compare comp) {
			if (first != last) {
				Iter next = first;
				while (++next != last) {
					if (comp(*next, *first)) {
						return false;
					}
					++first;
				}
			}
			return true;
		}

		template<typename Iter, typename Compare>
		constexpr bool is_sorted_unique(Iter first, Iter last, Compare comp) {
			if (first != last) {
				Iter next = first;
				while (++next != last) {
					if (!comp(*first, *next)) {
						return false;
					}
					++first;
				}
			}
			return true;
		}

		template<typename Pair, typename Compare>
		struct pair_compare : public Compare {
			pair_compare() = default;

			explicit pair_compare(const Compare& compare)
				: Compare(compare) {}

			bool operator()(
					const typename Pair::first_type& l,
					const typename Pair::first_type& r) const {
				return Compare::operator()(l, r);
			}

			bool operator()(const Pair& l, const Pair& r) const {
				return Compare::operator()(l.first, r.first);
			}

			bool operator()(
					const typename Pair::first_type& l,
					const Pair& r) const {
				return Compare::operator()(l, r.first);
			}

			bool operator()(
					const Pair& l,
					const typename Pair::first_type& r) const {
				return Compare::operator()(l.first, r);
			}

			template<typename K>
				requires (is_transparent_v<Compare, K>)
			bool operator()(const K& l, const Pair& r) const {
				return Compare::operator()(l, r.first);
			}

			template<typename K>
				requires (is_transparent_v<Compare, K>)
			bool operator()(const Pair& l, const K& r) const {
				return Compare::operator()(l.first, r);
			}
		};

		template<typename Compare>
		struct eq_compare : public Compare {
			eq_compare() = default;

			explicit eq_compare(const Compare& compare)
				: Compare(compare) {}

			template<typename L, typename R>
			bool operator()(const L& l, const R& r) const {
				return !Compare::operator()(l, r) && !Compare::operator()(r, l);
			}
		};
	} // namespace detail

	struct sorted_range_t {};
	inline constexpr sorted_range_t sorted_range = sorted_range_t();

	struct sorted_unique_range_t : public sorted_range_t {};
	inline constexpr sorted_unique_range_t sorted_unique_range = sorted_unique_range_t();

	template<typename Key, typename Value, typename Compare = std::less<Key>, typename Container = std::vector<std::pair<Key, Value>>>
	class flat_map {
	public:
		using key_type = Key;
		using mapped_type = Value;
		using value_type = typename Container::value_type;

		using size_type = typename Container::size_type;
		using difference_type = typename Container::difference_type;

		using key_compare = Compare;
		using container_type = Container;

		using reference = typename Container::reference;
		using const_reference = typename Container::const_reference;
		using pointer = typename Container::pointer;
		using const_pointer = typename Container::const_pointer;

		using iterator = typename Container::iterator;
		using const_iterator = typename Container::const_iterator;
		using reverse_iterator = typename Container::reverse_iterator;
		using const_reverse_iterator = typename Container::const_reverse_iterator;

		struct value_compare : private key_compare {
			value_compare() = default;

			explicit value_compare(key_compare compare)
				: key_compare(std::move(compare)) {}

			bool operator()(const value_type& l, const value_type& r) const {
				return key_compare::operator()(l.first, r.first);
			}
		};

	public:
		flat_map() = default;
		~flat_map() = default;

		explicit flat_map(const Compare& c)
			: _compare(c) {}

		template<typename Allocator>
		explicit flat_map(const Allocator& a)
			: _data(a) {}

		template<typename Allocator>
		flat_map(const Compare& c, const Allocator& a)
			: _compare(c), _data(a) {}

		template<std::input_iterator Iterator>
		flat_map(Iterator first, Iterator last) {
			from_range(first, last);
		}

		template<std::input_iterator Iterator>
		flat_map(sorted_range_t, Iterator first, Iterator last) {
			from_range(sorted_range, first, last);
		}

		template<std::input_iterator Iterator>
		flat_map(sorted_unique_range_t, Iterator first, Iterator last) {
			from_range(sorted_unique_range, first, last);
		}

		template<std::input_iterator Iterator>
		flat_map(Iterator first, Iterator last, const Compare& c)
			: _compare(c) {
			from_range(first, last);
		}

		template<std::input_iterator Iterator>
		flat_map(sorted_range_t, Iterator first, Iterator last, const Compare& c)
			: _compare(c) {
			from_range(sorted_range, first, last);
		}

		template<std::input_iterator Iterator>
		flat_map(sorted_unique_range_t, Iterator first, Iterator last, const Compare& c)
			: _compare(c) {
			from_range(sorted_unique_range, first, last);
		}

		template<std::input_iterator Iterator, typename Allocator>
		flat_map(Iterator first, Iterator last, const Allocator& a)
			: _data(a) {
			from_range(first, last);
		}

		template<std::input_iterator Iterator, typename Allocator>
		flat_map(sorted_range_t, Iterator first, Iterator last, const Allocator& a)
			: _data(a) {
			from_range(sorted_range, first, last);
		}

		template<std::input_iterator Iterator, typename Allocator>
		flat_map(sorted_unique_range_t, Iterator first, Iterator last, const Allocator& a)
			: _data(a) {
			from_range(sorted_unique_range, first, last);
		}

		template<std::input_iterator Iterator, typename Allocator>
		flat_map(Iterator first, Iterator last, const Compare& c, const Allocator& a)
			: _compare(c), _data(a) {
			from_range(first, last);
		}

		template<std::input_iterator Iterator, typename Allocator>
		flat_map(sorted_range_t, Iterator first, Iterator last, const Compare& c, const Allocator& a)
			: _compare(c), _data(a) {
			from_range(sorted_range, first, last);
		}

		template<std::input_iterator Iterator, typename Allocator>
		flat_map(sorted_unique_range_t, Iterator first, Iterator last, const Compare& c, const Allocator& a)
			: _compare(c), _data(a) {
			from_range(sorted_unique_range, first, last);
		}

		flat_map(std::initializer_list<value_type> list) {
			from_range(list.begin(), list.end());
		}

		flat_map(sorted_range_t, std::initializer_list<value_type> list) {
			from_range(sorted_range, list.begin(), list.end());
		}

		flat_map(sorted_unique_range_t, std::initializer_list<value_type> list) {
			from_range(sorted_unique_range, list.begin(), list.end());
		}

		flat_map(std::initializer_list<value_type> list, const Compare& c)
			: _compare(c) {
			from_range(list.begin(), list.end());
		}

		flat_map(sorted_range_t, std::initializer_list<value_type> list, const Compare& c)
			: _compare(c) {
			from_range(sorted_range, list.begin(), list.end());
		}

		flat_map(sorted_unique_range_t, std::initializer_list<value_type> list, const Compare& c)
			: _compare(c) {
			from_range(sorted_unique_range, list.begin(), list.end());
		}

		template<typename Allocator>
		flat_map(std::initializer_list<value_type> list, const Allocator& a)
			: _data(a) {
			from_range(list.begin(), list.end());
		}

		template<typename Allocator>
		flat_map(sorted_range_t, std::initializer_list<value_type> list, const Allocator& a)
			: _data(a) {
			from_range(sorted_range, list.begin(), list.end());
		}

		template<typename Allocator>
		flat_map(sorted_unique_range_t, std::initializer_list<value_type> list, const Allocator& a)
			: _data(a) {
			from_range(sorted_unique_range, list.begin(), list.end());
		}

		template<typename Allocator>
		flat_map(std::initializer_list<value_type> list, const Compare& c, const Allocator& a)
			: _compare(c), _data(a) {
			from_range(list.begin(), list.end());
		}

		template<typename Allocator>
		flat_map(sorted_range_t, std::initializer_list<value_type> list, const Compare& c, const Allocator& a)
			: _compare(c), _data(a) {
			from_range(sorted_range, list.begin(), list.end());
		}

		template<typename Allocator>
		flat_map(sorted_unique_range_t, std::initializer_list<value_type> list, const Compare& c, const Allocator& a)
			: _compare(c), _data(a) {
			from_range(sorted_unique_range, list.begin(), list.end());
		}

		template<typename Allocator>
		flat_map(flat_map&& other, const Allocator& a)
			: _compare(std::move(other._compare)), _data(std::move(other._data), a) {}

		template<typename Allocator>
		flat_map(const flat_map& other, const Allocator& a)
			: _compare(other._compare), _data(other._data, a) {}

		flat_map(flat_map&& other) noexcept = default;
		flat_map(const flat_map& other) = default;

		flat_map& operator=(flat_map&& other) noexcept = default;
		flat_map& operator=(const flat_map& other) = default;

		flat_map& operator=(std::initializer_list<value_type> list) {
			flat_map(list).swap(*this);
			return *this;
		}

		iterator begin() noexcept(noexcept(std::declval<container_type&>().begin())) {
			return _data.begin();
		}

		const_iterator begin() const
				noexcept(noexcept(std::declval<const container_type&>().begin())) {
			return _data.begin();
		}

		const_iterator cbegin() const
				noexcept(noexcept(std::declval<const container_type&>().cbegin())) {
			return _data.cbegin();
		}

		iterator end() noexcept(noexcept(std::declval<container_type&>().end())) {
			return _data.end();
		}

		const_iterator end() const
				noexcept(noexcept(std::declval<const container_type&>().end())) {
			return _data.end();
		}

		const_iterator cend() const
				noexcept(noexcept(std::declval<const container_type&>().cend())) {
			return _data.cend();
		}

		reverse_iterator rbegin() noexcept(noexcept(std::declval<container_type&>().rbegin())) {
			return _data.rbegin();
		}

		const_reverse_iterator rbegin() const
				noexcept(noexcept(std::declval<const container_type&>().rbegin())) {
			return _data.rbegin();
		}

		const_reverse_iterator crbegin() const
				noexcept(noexcept(std::declval<const container_type&>().crbegin())) {
			return _data.crbegin();
		}

		reverse_iterator rend() noexcept(noexcept(std::declval<container_type&>().rend())) {
			return _data.rend();
		}

		const_reverse_iterator rend() const
				noexcept(noexcept(std::declval<const container_type&>().rend())) {
			return _data.rend();
		}

		const_reverse_iterator crend() const
				noexcept(noexcept(std::declval<const container_type&>().crend())) {
			return _data.crend();
		}

		bool empty() const
				noexcept(noexcept(std::declval<const container_type&>().empty())) {
			return _data.empty();
		}

		size_type size() const
				noexcept(noexcept(std::declval<const container_type&>().size())) {
			return _data.size();
		}

		size_type max_size() const
				noexcept(noexcept(std::declval<const container_type&>().max_size())) {
			return _data.max_size();
		}

		size_type capacity() const
				noexcept(noexcept(std::declval<const container_type&>().capacity())) {
			return _data.capacity();
		}

		void reserve(size_type capacity) {
			_data.reserve(capacity);
		}

		void shrink_to_fit() {
			_data.shrink_to_fit();
		}

		mapped_type& operator[](key_type&& key) {
			const iterator iter = find(key);
			return iter != end()
						   ? iter->second
						   : emplace(std::move(key), mapped_type()).first->second;
		}

		mapped_type& operator[](const key_type& key) {
			const iterator iter = find(key);
			return iter != end()
						   ? iter->second
						   : emplace(key, mapped_type()).first->second;
		}

		mapped_type& at(const key_type& key) {
			const iterator iter = find(key);
			PLUGIFY_ASSERT(iter != end(), "plg::flat_map::at(): key not found", std::out_of_range);
			return iter->second;
		}

		const mapped_type& at(const key_type& key) const {
			const const_iterator iter = find(key);
			PLUGIFY_ASSERT(iter != end(), "plg::flat_map::at(): key not found", std::out_of_range);
			return iter->second;
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		mapped_type& at(const K& key) {
			const iterator iter = find(key);
			PLUGIFY_ASSERT(iter != end(), "plg::flat_map::at(): key not found", std::out_of_range);
			return iter->second;
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		const mapped_type& at(const K& key) const {
			const const_iterator iter = find(key);
			PLUGIFY_ASSERT(iter != end(), "plg::flat_map::at(): key not found", std::out_of_range);
			return iter->second;
		}

		std::pair<iterator, bool> insert(value_type&& value) {
			const iterator iter = lower_bound(value.first);
			return iter == end() || _compare(value, *iter)
						   ? std::make_pair(_data.insert(iter, std::move(value)), true)
						   : std::make_pair(iter, false);
		}

		std::pair<iterator, bool> insert(const value_type& value) {
			const iterator iter = lower_bound(value.first);
			return iter == end() || _compare(value, *iter)
						   ? std::make_pair(_data.insert(iter, value), true)
						   : std::make_pair(iter, false);
		}

		iterator insert(const_iterator hint, value_type&& value) {
			return (hint == begin() || _compare(*(hint - 1), value)) && (hint == end() || _compare(value, *hint))
						   ? _data.insert(hint, std::move(value))
						   : insert(std::move(value)).first;
		}

		iterator insert(const_iterator hint, const value_type& value) {
			return (hint == begin() || _compare(*(hint - 1), value)) && (hint == end() || _compare(value, *hint))
						   ? _data.insert(hint, value)
						   : insert(value).first;
		}

		template<typename V>
		std::pair<iterator, bool> insert_or_assign(key_type&& key, V&& value) {
			iterator iter = lower_bound(key);
			if (iter == end() || _compare(key, *iter)) {
				iter = emplace_hint(iter, std::move(key), std::forward<V>(value));
				return {iter, true};
			}
			(*iter).second = std::forward<V>(value);
			return {iter, false};
		}

		template<typename V>
		std::pair<iterator, bool> insert_or_assign(const key_type& key, V&& value) {
			iterator iter = lower_bound(key);
			if (iter == end() || _compare(key, *iter)) {
				iter = emplace_hint(iter, key, std::forward<V>(value));
				return {iter, true};
			}
			(*iter).second = std::forward<V>(value);
			return {iter, false};
		}

		template<std::input_iterator Iterator>
		void insert(Iterator first, Iterator last) {
			insert_range(first, last);
		}

		template<std::input_iterator Iterator>
		void insert(sorted_range_t, Iterator first, Iterator last) {
			insert_range(sorted_range, first, last);
		}

		void insert(std::initializer_list<value_type> list) {
			insert_range(list.begin(), list.end());
		}

		void insert(sorted_range_t, std::initializer_list<value_type> list) {
			insert_range(sorted_range, list.begin(), list.end());
		}

		template<typename... Args>
		std::pair<iterator, bool> emplace(Args&&... args) {
			return insert(value_type(std::forward<Args>(args)...));
		}

		template<typename... Args>
		iterator emplace_hint(const_iterator hint, Args&&... args) {
			return insert(hint, value_type(std::forward<Args>(args)...));
		}

		template<typename... Args>
		std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) {
			iterator iter = lower_bound(key);
			if (iter == end() || _compare(key, *iter)) {
				iter = emplace_hint(iter, std::move(key), std::forward<Args>(args)...);
				return {iter, true};
			}
			return {iter, false};
		}

		template<typename... Args>
		std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
			iterator iter = lower_bound(key);
			if (iter == end() || _compare(key, *iter)) {
				iter = emplace_hint(iter, key, std::forward<Args>(args)...);
				return {iter, true};
			}
			return {iter, false};
		}

		void clear() noexcept(noexcept(std::declval<container_type&>().clear())) {
			_data.clear();
		}

		iterator erase(const_iterator iter) {
			return _data.erase(iter);
		}

		iterator erase(const_iterator first, const_iterator last) {
			return _data.erase(first, last);
		}

		size_type erase(const key_type& key) {
			const const_iterator iter = find(key);
			return iter != end()
						   ? (erase(iter), 1)
						   : 0;
		}

		void swap(flat_map& other) noexcept(std::is_nothrow_swappable_v<Compare> && std::is_nothrow_swappable_v<container_type>) {
			using std::swap;
			swap(_compare, other._compare);
			swap(_data, other._data);
		}

		size_type count(const key_type& key) const {
			const const_iterator iter = find(key);
			return iter != end() ? 1 : 0;
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		size_type count(const K& key) const {
			const const_iterator iter = find(key);
			return iter != end() ? 1 : 0;
		}

		iterator find(const key_type& key) {
			const iterator iter = lower_bound(key);
			return iter != end() && !_compare(key, *iter)
						   ? iter
						   : end();
		}

		const_iterator find(const key_type& key) const {
			const const_iterator iter = lower_bound(key);
			return iter != end() && !_compare(key, *iter)
						   ? iter
						   : end();
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		iterator find(const K& key) {
			const iterator iter = lower_bound(key);
			return iter != end() && !_compare(key, *iter)
						   ? iter
						   : end();
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		const_iterator find(const K& key) const {
			const const_iterator iter = lower_bound(key);
			return iter != end() && !_compare(key, *iter)
						   ? iter
						   : end();
		}

		bool contains(const key_type& key) const {
			return find(key) != end();
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		bool contains(const K& key) const {
			return find(key) != end();
		}

		std::pair<iterator, iterator> equal_range(const key_type& key) {
			return std::equal_range(begin(), end(), key, _compare);
		}

		std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const {
			return std::equal_range(begin(), end(), key, _compare);
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		std::pair<iterator, iterator> equal_range(const K& key) {
			return std::equal_range(begin(), end(), key, _compare);
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		std::pair<const_iterator, const_iterator> equal_range(const K& key) const {
			return std::equal_range(begin(), end(), key, _compare);
		}

		iterator lower_bound(const key_type& key) {
			return std::lower_bound(begin(), end(), key, _compare);
		}

		const_iterator lower_bound(const key_type& key) const {
			return std::lower_bound(begin(), end(), key, _compare);
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		iterator lower_bound(const K& key) {
			return std::lower_bound(begin(), end(), key, _compare);
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		const_iterator lower_bound(const K& key) const {
			return std::lower_bound(begin(), end(), key, _compare);
		}

		iterator upper_bound(const key_type& key) {
			return std::upper_bound(begin(), end(), key, _compare);
		}

		const_iterator upper_bound(const key_type& key) const {
			return std::upper_bound(begin(), end(), key, _compare);
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		iterator upper_bound(const K& key) {
			return std::upper_bound(begin(), end(), key, _compare);
		}

		template<typename K>
			requires (detail::is_transparent_v<Compare, K>)
		const_iterator upper_bound(const K& key) const {
			return std::upper_bound(begin(), end(), key, _compare);
		}

		key_compare key_comp() const {
			return _compare;
		}

		value_compare value_comp() const {
			return value_compare(key_comp());
		}

	private:
		template<typename Iter>
		void from_range(Iter first, Iter last) {
			assert(_data.empty());
			_data.insert(_data.end(), first, last);
			std::sort(_data.begin(), _data.end(), value_comp());
			_data.erase(
					std::unique(_data.begin(), _data.end(),
								detail::eq_compare<value_compare>(value_comp())),
					_data.end());
		}

		template<typename Iter>
		void from_range(sorted_range_t, Iter first, Iter last) {
			assert(_data.empty());
			assert(detail::is_sorted(first, last, value_comp()));
			_data.insert(_data.end(), first, last);
			_data.erase(
					std::unique(_data.begin(), _data.end(),
								detail::eq_compare<value_compare>(value_comp())),
					_data.end());
		}

		template<typename Iter>
		void from_range(sorted_unique_range_t, Iter first, Iter last) {
			assert(_data.empty());
			assert(detail::is_sorted_unique(first, last, value_comp()));
			_data.insert(_data.end(), first, last);
		}

	private:
		template<typename Iter>
		void insert_range(Iter first, Iter last) {
			const auto mid_iter = _data.insert(_data.end(), first, last);
			std::sort(mid_iter, _data.end(), value_comp());
			std::inplace_merge(_data.begin(), mid_iter, _data.end(), value_comp());
			_data.erase(
					std::unique(_data.begin(), _data.end(),
								detail::eq_compare<value_compare>(value_comp())),
					_data.end());
		}

		template<typename Iter>
		void insert_range(sorted_range_t, Iter first, Iter last) {
			assert(detail::is_sorted(first, last, value_comp()));
			const auto mid_iter = _data.insert(_data.end(), first, last);
			std::inplace_merge(_data.begin(), mid_iter, _data.end(), value_comp());
			_data.erase(
					std::unique(_data.begin(), _data.end(),
								detail::eq_compare<value_compare>(value_comp())),
					_data.end());
		}

	private:
		PLUGIFY_NO_UNIQUE_ADDRESS
		detail::pair_compare<value_type, key_compare> _compare;
		container_type _data;
	};

	template<typename Key, typename Value, typename Compare, typename Container>
	void swap(
			flat_map<Key, Value, Compare, Container>& l,
			flat_map<Key, Value, Compare, Container>& r) noexcept(noexcept(l.swap(r))) {
		l.swap(r);
	}

	template<typename Key, typename Value, typename Compare, typename Container>
	bool operator==(
			const flat_map<Key, Value, Compare, Container>& l,
			const flat_map<Key, Value, Compare, Container>& r) {
		return l.size() == r.size() && std::equal(l.begin(), l.end(), r.begin());
	}

	template<typename Key, typename Value, typename Compare, typename Container>
	auto operator<=>(
			const flat_map<Key, Value, Compare, Container>& l,
			const flat_map<Key, Value, Compare, Container>& r) {
		if (l.size() < r.size()) {
			return std::partial_ordering::less;
		} else if (l.size() > r.size()) {
			return std::partial_ordering::greater;
		} else {
			if (std::lexicographical_compare(l.cbegin(), l.cend(), r.cbegin(), r.cend())) {
				return std::partial_ordering::less;
			} else {
				return std::partial_ordering::greater;
			}
		}
	}

	template<typename Key, typename Value, typename Compare = std::less<Key>, typename Container = plg::vector<std::pair<Key, Value>>>
	using map = flat_map<Key, Value, Compare, Container>;

}// namespace plg
#endif