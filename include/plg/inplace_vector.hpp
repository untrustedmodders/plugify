#pragma once

#include "plg/macro.hpp"

#if __has_include(<inplace_vector>)
#include <inplace_vector>
#if defined(__cpp_lib_inplace_vector) && __cpp_lib_inplace_vector >= 202406L	
#define PLUGIFY_HAS_STD_INPLACE_VECTOR 1
#else
#define PLUGIFY_HAS_STD_INPLACE_VECTOR 0
#endif
#else
#define PLUGIFY_HAS_STD_INPLACE_VECTOR 0
#endif

#if !PLUGIFY_HAS_STD_INPLACE_VECTOR
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>
#if PLUGIFY_CPP_VERSION >= 202002L
# include <compare>
# include <ranges>
#endif

// from https://github.com/TedLyngmo/inplace_vector
namespace plg {
	template<class, std::size_t>
	class inplace_vector;

	namespace detail {
		using std::is_nothrow_swappable;
		template<class... B>
		using conjunction = std::conjunction<B...>;

		// base requirements
		template<class T, std::size_t N>
		struct constexpr_compat :
			std::integral_constant<bool,
								   N == 0 || (std::is_trivially_default_constructible_v<T> && std::is_trivially_copyable_v<T>
											  /* the four biggest implementations agree that the combination of the two above implies:
											  && std::is_trivially_copy_constructible_v<T>
											  && std::is_trivially_move_constructible_v<T>
											  && std::is_trivially_copy_assignable_v<T>
											  && std::is_trivially_move_assignable_v<T>
											  && std::is_trivially_destructible_v<T> */
											  )> {};

		template<class T, std::size_t N>
		struct trivial_copy_ctor : std::integral_constant<bool, N == 0 || std::is_trivially_copy_constructible_v<T>> {};

		template<class T, std::size_t N>
		struct trivial_move_ctor : std::integral_constant<bool, N == 0 || std::is_trivially_move_constructible_v<T>> {};

		template<class T, std::size_t N>
		struct trivial_copy_ass :
			std::integral_constant<bool, N == 0 || (std::is_trivially_destructible_v<T> &&
													std::is_trivially_copy_constructible_v<T> &&
													std::is_trivially_copy_assignable_v<T>)> {};

		template<class T, std::size_t N>
		struct trivial_move_ass :
			std::integral_constant<bool, N == 0 || (std::is_trivially_destructible_v<T> &&
													std::is_trivially_move_constructible_v<T> &&
													std::is_trivially_move_assignable_v<T>)> {};

		template<class T>
		struct aligned_storage_empty { // specialization for 0 elements
			using value_type = std::remove_const_t<T>;
			using size_type = std::size_t;
			using reference = value_type&;
			using const_reference = value_type const&;
			using pointer = value_type*;
			using const_pointer = value_type const*;

		protected:
			constexpr pointer ptr(size_type) { return nullptr; }
			constexpr const_pointer ptr(size_type) const { return nullptr; }
			constexpr reference ref(size_type) { return *ptr(0); }
			constexpr const_reference ref(size_type) const { return *ptr(0); }

			template<class... Args>
			constexpr reference construct_back(Args&&...) {
				return *ptr(0);
			}
			constexpr void destroy(size_type) {}

			constexpr reference operator[](size_type) { return *ptr(0); }
			constexpr const_reference operator[](size_type) const { return *ptr(0); }

			constexpr size_type size() const noexcept { return 0; }
			constexpr void clear() noexcept {}

			[[maybe_unused]] constexpr size_type inc() { return 0; }
			[[maybe_unused]] constexpr size_type dec(size_type = 1) { return 0; }
		};

		template<class T, std::size_t N>
		struct aligned_storage_trivial {
			static_assert(std::is_trivially_destructible_v<T>, "T must be trivially destructible");

			using value_type = std::remove_const_t<T>;
			using size_type = std::size_t;
			using reference = value_type&;
			using const_reference = value_type const&;
			using pointer = value_type*;
			using const_pointer = value_type const*;

		protected:
			constexpr pointer ptr(size_type idx) noexcept { return std::addressof(_data[idx]); }
			constexpr const_pointer ptr(size_type idx) const noexcept { return std::addressof(_data[idx]); }
			constexpr reference ref(size_type idx) noexcept { return _data[idx]; }
			constexpr const_reference ref(size_type idx) const noexcept { return _data[idx]; }

			template<class... Args>
			constexpr reference construct_back(Args&&... args) {
				auto& rv = _data[_size] = value_type{std::forward<Args>(args)...};
				++_size;
				return rv;
			}
			constexpr void destroy(size_type) noexcept {}

			constexpr reference operator[](size_type idx) noexcept { return ref(idx); }
			constexpr const_reference operator[](size_type idx) const noexcept { return ref(idx); }

			constexpr size_type size() const noexcept { return _size; }
			constexpr void clear() noexcept { _size = 0; }

			[[maybe_unused]] constexpr size_type inc() noexcept { return ++_size; }
			[[maybe_unused]] constexpr size_type dec(size_type count = 1) noexcept { return _size -= count; }

		private:
			std::array<value_type, N> _data;
			size_type _size = 0;
		};

		template<class T, std::size_t N>
		struct aligned_storage_non_trivial {
			using value_type = std::remove_const_t<T>;
			using size_type = std::size_t;
			using reference = value_type&;
			using const_reference = value_type const&;
			using pointer = value_type*;
			using const_pointer = value_type const*;

		protected:
			constexpr pointer ptr(size_type idx) noexcept { return _data[idx].ptr(); }
			constexpr const_pointer ptr(size_type idx) const noexcept { return _data[idx].ptr(); }
			constexpr reference ref(size_type idx) noexcept { return *ptr(idx); }
			constexpr const_reference ref(size_type idx) const noexcept { return *ptr(idx); }

			template<class... Args>
			constexpr reference construct_back(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) {
				auto& rv = *std::construct_at(ptr(_size), std::forward<Args>(args)...);
				++_size;
				return rv;
			}
			constexpr void destroy(size_type idx) noexcept { ref(idx).~T(); }

			constexpr reference operator[](size_type idx) noexcept { return ref(idx); }
			constexpr const_reference operator[](size_type idx) const noexcept { return ref(idx); }

			constexpr size_type size() const noexcept { return _size; }

			[[maybe_unused]] constexpr size_type inc() noexcept { return ++_size; }
			[[maybe_unused]] constexpr size_type dec(size_type count = 1) noexcept { return _size -= count; }
			constexpr void clear() noexcept(std::is_nothrow_destructible_v<T>) {
				while (_size) {
					destroy(--_size);
				}
			}

		private:
			struct alignas(T) inner_storage {
				std::array<unsigned char, sizeof(T)> data;
				pointer ptr() noexcept { return std::launder(reinterpret_cast<pointer>(data.data())); }
				const_pointer ptr() const noexcept { return std::launder(reinterpret_cast<const_pointer>(data.data())); }
			};

			std::array<inner_storage, N> _data;
			static_assert(sizeof _data == sizeof(T[N]), "erroneous size");
			size_type _size = 0;
		};

		template<class T, std::size_t N>
		struct storage_selector :
			std::conditional_t<constexpr_compat<T, N>::value, aligned_storage_trivial<T, N>, aligned_storage_non_trivial<T, N>> {};

		template<class T, std::size_t N>
		struct non_trivial_destructor : storage_selector<T, N> {
			constexpr non_trivial_destructor() = default;
			constexpr non_trivial_destructor(const non_trivial_destructor&) = default;
			constexpr non_trivial_destructor(non_trivial_destructor&&) noexcept = default;
			constexpr non_trivial_destructor& operator=(const non_trivial_destructor&) = default;
			constexpr non_trivial_destructor& operator=(non_trivial_destructor&&) noexcept = default;
			constexpr ~non_trivial_destructor() noexcept(std::is_nothrow_destructible_v<T>) {
				this->clear();
			}
		};
		template<class T, std::size_t N>
		struct dtor_selector :
			std::conditional_t<std::is_trivially_destructible_v<T>, storage_selector<T, N>, non_trivial_destructor<T, N>> {};

		template<class T, std::size_t N>
		struct non_trivial_copy_ass : dtor_selector<T, N> {
			constexpr non_trivial_copy_ass() = default;
			constexpr non_trivial_copy_ass(const non_trivial_copy_ass&) = default;
			constexpr non_trivial_copy_ass(non_trivial_copy_ass&&) noexcept = default;
			constexpr non_trivial_copy_ass& operator=(const non_trivial_copy_ass& other) {
				this->clear(); // may copy assign std::min(size(), other.size()) elements
				for (decltype(this->size()) idx = 0; idx != other.size(); ++idx) {
					this->construct_back(other.ref(idx));
				}
				return *this;
			}
			constexpr non_trivial_copy_ass& operator=(non_trivial_copy_ass&&) noexcept = default;
			constexpr ~non_trivial_copy_ass() = default;
		};
		template<class T, std::size_t N>
		struct copy_ass_selector :
			std::conditional_t<trivial_copy_ass<T, N>::value, dtor_selector<T, N>, non_trivial_copy_ass<T, N>> {};

		template<class T, std::size_t N>
		struct non_trivial_move_ass : copy_ass_selector<T, N> {
			constexpr non_trivial_move_ass() = default;
			constexpr non_trivial_move_ass(const non_trivial_move_ass&) = default;
			constexpr non_trivial_move_ass(non_trivial_move_ass&&) noexcept = default;
			constexpr non_trivial_move_ass& operator=(const non_trivial_move_ass&) = default;
			constexpr non_trivial_move_ass& operator=(non_trivial_move_ass&& other) noexcept(
				std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
				this->clear(); // may move assign std::min(size(), other.size()) elements
				for (decltype(this->size()) idx = 0; idx != other.size(); ++idx) {
					this->construct_back(std::move(other.ref(idx)));
				}
				other.clear();
				return *this;
			}
		};
		template<class T, std::size_t N>
		struct move_ass_selector :
			std::conditional_t<trivial_move_ass<T, N>::value, copy_ass_selector<T, N>, non_trivial_move_ass<T, N>> {};

		template<class T, std::size_t N>
		struct non_trivial_copy_ctor : move_ass_selector<T, N> {
			constexpr non_trivial_copy_ctor() = default;
			constexpr non_trivial_copy_ctor(const non_trivial_copy_ctor& other) {
				for (decltype(this->size()) idx = 0; idx != other.size(); ++idx) {
					this->construct_back(other.ref(idx));
				}
			}
			constexpr non_trivial_copy_ctor(non_trivial_copy_ctor&&) noexcept = default;
			constexpr non_trivial_copy_ctor& operator=(const non_trivial_copy_ctor& other) = default;
			constexpr non_trivial_copy_ctor& operator=(non_trivial_copy_ctor&&) noexcept = default;
			constexpr ~non_trivial_copy_ctor() = default;
		};
		template<class T, std::size_t N>
		struct copy_ctor_selector :
			std::conditional_t<trivial_copy_ctor<T, N>::value, move_ass_selector<T, N>, non_trivial_copy_ctor<T, N>> {};

		template<class T, std::size_t N>
		struct non_trivial_move_ctor : copy_ctor_selector<T, N> {
			constexpr non_trivial_move_ctor() = default;
			constexpr non_trivial_move_ctor(const non_trivial_move_ctor&) = default;
			constexpr non_trivial_move_ctor(non_trivial_move_ctor&& other) noexcept(
				std::is_nothrow_move_constructible_v<T>) {
				for (decltype(this->size()) idx = 0; idx != other.size(); ++idx) {
					this->construct_back(std::move(other.ref(idx)));
				}
				other.clear();
			}
			constexpr non_trivial_move_ctor& operator=(const non_trivial_move_ctor& other) = default;
			constexpr non_trivial_move_ctor& operator=(non_trivial_move_ctor&&) noexcept = default;
			constexpr ~non_trivial_move_ctor() = default;
		};
		template<class T, std::size_t N>
		struct move_ctor_selector :
			std::conditional_t<trivial_move_ctor<T, N>::value, copy_ctor_selector<T, N>, non_trivial_move_ctor<T, N>> {};

		template<class T, std::size_t N>
		struct base_selector : std::conditional_t<N == 0, aligned_storage_empty<T>, move_ctor_selector<T, N>> {};
	} // namespace detail

	template<class T, std::size_t N>
	class inplace_vector : public detail::base_selector<T, N> {
		static_assert(std::is_nothrow_destructible_v<T>,
					  "inplace_vector: classes with potentially throwing destructors are prohibited");
		using base = detail::base_selector<T, N>;
		using base::construct_back;
		using base::dec;
		using base::destroy;
		using base::ptr;
		using base::ref;

	public:
		using base::size;
		using base::operator[];
		using base::clear;

		using value_type = T;
		using size_type = std::size_t;
		using reference = T&;
		using const_reference = T const&;
		using pointer = T*;
		using const_pointer = T const*;
		using iterator = T*;
		using const_iterator = T const*;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using difference_type = typename std::iterator_traits<iterator>::difference_type;

	private:
		constexpr void shrink_to(const size_type count) noexcept {
			while (count != size()) {
				pop_back();
			}
		}

	public:
		// Constructors with concepts
		constexpr inplace_vector() noexcept = default;

		// Instead of: explicit inplace_vector(size_type count) requires std::default_initializable<T>
		constexpr explicit inplace_vector(size_type count)
			requires std::default_initializable<T>
		{
			PLUGIFY_ASSERT(count <= N, "resulted vector size would exceed capacity()", std::bad_alloc);
			while (count != size()) unchecked_emplace_back();
		}

		constexpr inplace_vector(size_type count, const T& value)
			requires std::copy_constructible<T>
		{
			PLUGIFY_ASSERT(count <= N, "resulted vector size would exceed capacity()", std::bad_alloc);
			while (count != size()) unchecked_push_back(value);
		}

		template<class InputIt>
			requires std::constructible_from<T, typename std::iterator_traits<InputIt>::value_type>
		constexpr inplace_vector(InputIt first, InputIt last) {
			std::copy(first, last, std::back_inserter(*this));
		}

		constexpr inplace_vector(std::initializer_list<T> init)
			requires std::copy_constructible<T>
		: inplace_vector(init.begin(), init.end()) {}

#if PLUGIFY_INPLACE_VECTOR_CONTAINERS_RANGES
		template<detail::container_compatiblel_range<T> R>
		constexpr inplace_vector(std::fro_range_t, R&& rg) {
			if constexpr(std::ranges::sized_range<R>) {
				PLUGIFY_ASSERT(std::ranges::size(rg) <= N, "resulted vector size would exceed capacity()", std::bad_alloc);
				for (auto&& val : rg) unchecked_emplace_back(std::forward<decltype(val)>(val));
			} else {
				for (auto&& val : rg) emplace_back(std::forward<decltype(val)>(val));
			}
		}
#endif

		// Assignment operators with concepts
		constexpr inplace_vector& operator=(std::initializer_list<T> init)
			requires std::is_copy_constructible_v<T>
		{
			PLUGIFY_ASSERT(init.size() <= capacity(), "resulted vector size would exceed capacity()", std::bad_alloc);
			assign(init.begin(), init.end());
			return *this;
		}

		constexpr void assign(size_type count, const T& value)
			requires std::is_copy_constructible_v<T>
		{
			PLUGIFY_ASSERT(count <= capacity(), "resulted vector size would exceed capacity()", std::bad_alloc);
			clear();
			while (count != size()) push_back(value);
		}

		template<class InputIt>
			requires std::is_constructible_v<T, typename std::iterator_traits<InputIt>::value_type>
		constexpr void assign(InputIt first, InputIt last) {
			clear();
			std::copy(first, last, std::back_inserter(*this));
		}

		constexpr void assign(std::initializer_list<T> ilist)
			requires std::is_copy_constructible_v<T>
		{
			PLUGIFY_ASSERT(ilist.size() <= capacity(), "resulted vector size would exceed capacity()", std::bad_alloc);
			clear();
			std::copy(ilist.begin(), ilist.end(), std::back_inserter(*this));
		}

#if PLUGIFY_INPLACE_VECTOR_CONTAINERS_RANGES
		template<detail::container_compatiblel_range<T> R>
		constexpr void assign_range(R&& rg)
			requires std::constructible_from<T&, std::ranges::range_reference_t<R>>
		{
			clear();
			append_range(std::forward<R>(rg));
		}

		template<detail::container_compatiblel_range<T> R>
		constexpr void append_range(R&& rg)
			requires std::constructible_from<T&, std::ranges::range_reference_t<R>>
		{
			if constexpr(std::ranges::sized_range<R>) {
				PLUGIFY_ASSERT(size() + std::ranges::size(rg) <= capacity(), "resulted vector size would exceed capacity()", std::bad_alloc);
				for (auto&& val : rg) {
					unchecked_emplace_back(std::forward<decltype(val)>(val));
				}
			} else {
				for (auto&& val : rg) {
					emplace_back(std::forward<decltype(val)>(val));
				}
			}
		}

		template<detail::container_compatiblel_range<T> R>
		constexpr std::ranges::borrowed_iterator_t<R> try_append_range(R&& rg)
			requires std::constructible_from<T&, std::ranges::range_reference_t<R>>
		{
			auto it = std::ranges::begin(rg);
			for (auto end = std::ranges::end(rg); it != end; std::ranges::advance(it, 1)) {
				if (size() == capacity()) break;
				unchecked_emplace_back(*it);
			}
			return it;
		}
#endif

		// Element access (no changes needed)
		constexpr reference at(size_type idx) {
			PLUGIFY_ASSERT(idx < size(), "input index is out of bounds", std::out_of_range);
			return ref(idx);
		}
		constexpr const_reference at(size_type idx) const {
			PLUGIFY_ASSERT(idx < size(), "input index is out of bounds", std::out_of_range);
			return ref(idx);
		}
		constexpr reference front() noexcept { return ref(0); }
		constexpr const_reference front() const noexcept { return ref(0); }
		constexpr reference back() noexcept { return ref(size() - 1); }
		constexpr const_reference back() const noexcept { return ref(size() - 1); }

		constexpr pointer data() noexcept { return ptr(0); }
		constexpr const_pointer data() const noexcept { return ptr(0); }

		// Iterators (no changes needed)
		constexpr const_iterator cbegin() const noexcept { return data(); }
		constexpr const_iterator cend() const noexcept { return std::next(cbegin(), static_cast<difference_type>(size())); }
		constexpr const_iterator begin() const noexcept { return cbegin(); }
		constexpr const_iterator end() const noexcept { return cend(); }
		constexpr iterator begin() noexcept { return data(); }
		constexpr iterator end() noexcept { return std::next(begin(), static_cast<difference_type>(size())); }

		constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
		constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }
		constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
		constexpr const_reverse_iterator rend() const noexcept { return crend(); }
		constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
		constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

		// Size and capacity (no changes needed)
		constexpr bool empty() const noexcept { return size() == 0; }
		static constexpr size_type max_size() noexcept { return N; }
		static constexpr size_type capacity() noexcept { return N; }

	private:
		constexpr void unchecked_resize(size_type count)
			requires std::is_default_constructible_v<T>
		{
			if (count < size()) {
				shrink_to(count);
			} else {
				while (count != size()) {
					unchecked_emplace_back();
				}
			}
		}

		constexpr void unchecked_resize(size_type count, const value_type& value)
			requires std::is_copy_constructible_v<T>
		{
			if (count < size()) {
				shrink_to(count);
			} else {
				while (count != size()) {
					unchecked_push_back(value);
				}
			}
		}

	public:
		constexpr void resize(size_type count)
			requires std::is_default_constructible_v<T>
		{
			PLUGIFY_ASSERT(count <= capacity(), "resulted vector size would exceed capacity()", std::bad_alloc);
			unchecked_resize(count);
		}

		constexpr void resize(size_type count, const value_type& value)
			requires std::is_copy_constructible_v<T>
		{
			PLUGIFY_ASSERT(count <= capacity(), "resulted vector size would exceed capacity()", std::bad_alloc);
			unchecked_resize(count, value);
		}

		static constexpr void reserve(size_type new_cap) {
			PLUGIFY_ASSERT(new_cap <= capacity(), "resulted vector size would exceed capacity()", std::bad_alloc);
		}
		static constexpr void shrink_to_fit() noexcept {}

		// Modifiers with concepts
		constexpr iterator insert(const_iterator pos, const T& value)
			requires std::is_copy_constructible_v<T>
		{
			PLUGIFY_ASSERT(size() != capacity(), "resulted vector size would exceed capacity()", std::bad_alloc);
			const auto ncpos = const_cast<iterator>(pos);
			unchecked_push_back(value);
			std::rotate(ncpos, std::prev(end()), end());
			return ncpos;
		}

		constexpr iterator insert(const_iterator pos, T&& value)
			requires std::is_move_constructible_v<T>
		{
			PLUGIFY_ASSERT(size() != capacity(), "resulted vector size would exceed capacity()", std::bad_alloc);
			const auto ncpos = const_cast<iterator>(pos);
			unchecked_push_back(std::move(value));
			std::rotate(ncpos, std::prev(end()), end());
			return ncpos;
		}

		constexpr iterator insert(const_iterator pos, size_type count, const T& value)
			requires std::is_copy_constructible_v<T>
		{
			PLUGIFY_ASSERT(size() + count <= capacity(), "resulted vector size would exceed capacity()", std::bad_alloc);
			const auto ncpos = const_cast<iterator>(pos);
			auto oldsize = size();
			auto first_inserted = end();
			try {
				while (count--) {
					unchecked_push_back(value);
				}
			} catch(...) {
				shrink_to(oldsize);
				throw;
			}
			std::rotate(ncpos, first_inserted, end());
			return ncpos;
		}

		template<class InputIt>
			requires (std::is_constructible_v<T, typename std::iterator_traits<InputIt>::value_type> &&
					 !std::is_const_v<T>)
		constexpr iterator insert(const_iterator pos, InputIt first, InputIt last) {
			const auto ncpos = const_cast<iterator>(pos);
			auto oldsize = size();
			auto first_inserted = end();
			try {
				for (; first != last; std::advance(first, 1)) {
					push_back(*first);
				}
			} catch(...) {
				shrink_to(oldsize);
				throw;
			}
			std::rotate(ncpos, first_inserted, end());
			return ncpos;
		}

		constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist)
			requires (std::is_copy_constructible_v<T> && !std::is_const_v<T>)
		{
			return insert(pos, ilist.begin(), ilist.end());
		}

		template<class... Args>
			requires std::is_constructible_v<T, Args...>
		constexpr iterator emplace(const_iterator pos, Args&&... args) {
			const auto ncpos = const_cast<iterator>(pos);
			emplace_back(std::forward<Args>(args)...);
			std::rotate(ncpos, std::prev(end()), end());
			return ncpos;
		}

		template<class... Args>
			requires std::is_constructible_v<T, Args...>
		constexpr reference unchecked_emplace_back(Args&&... args) {
			return construct_back(std::forward<Args>(args)...);
		}

		constexpr reference unchecked_push_back(T const& value)
			requires std::is_copy_constructible_v<T>
		{
			return construct_back(value);
		}

		constexpr reference unchecked_push_back(T&& value)
			requires std::is_move_constructible_v<T>
		{
			return construct_back(std::move(value));
		}

		template<class... Args>
			requires std::is_constructible_v<T, Args...>
		constexpr reference emplace_back(Args&&... args) {
			PLUGIFY_ASSERT(size() != N, "resulted vector size would exceed capacity()", std::bad_alloc);
			return unchecked_emplace_back(std::forward<Args>(args)...);
		}

		template<class... Args>
			requires std::is_constructible_v<T, Args...>
		constexpr pointer try_emplace_back(Args&&... args) {
			if (size() == N) return nullptr;
			return std::addressof(unchecked_emplace_back(std::forward<Args>(args)...));
		}

		constexpr reference push_back(T const& value)
			requires std::is_copy_constructible_v<T>
		{
			PLUGIFY_ASSERT(size() != N, "resulted vector size would exceed capacity()", std::bad_alloc);
			return unchecked_push_back(value);
		}

		constexpr reference push_back(T&& value)
			requires std::is_move_constructible_v<T>
		{
			PLUGIFY_ASSERT(size() != N, "resulted vector size would exceed capacity()", std::bad_alloc);
			return unchecked_push_back(std::move(value));
		}

		constexpr pointer try_push_back(T const& value)
			requires std::is_copy_constructible_v<T>
		{
			if (size() == N) return nullptr;
			return std::addressof(unchecked_push_back(value));
		}

		constexpr pointer try_push_back(T&& value)
			requires std::is_move_constructible_v<T>
		{
			if (size() == N) return nullptr;
			return std::addressof(unchecked_push_back(std::move(value)));
		}

		constexpr void pop_back() noexcept { destroy(dec()); }

		constexpr iterator erase(const_iterator first, const_iterator last)
			requires (!std::is_const_v<T>)
		{
			auto ncfirst = const_cast<iterator>(first);
			auto nclast = const_cast<iterator>(last);
			auto removed = static_cast<std::size_t>(std::distance(ncfirst, nclast));
			std::move(nclast, end(), ncfirst);
			for (size_type idx = size() - removed; idx < size(); ++idx) {
				destroy(idx);
			}
			dec(removed);
			return ncfirst;
		}

		constexpr iterator erase(const_iterator pos)
			requires (!std::is_const_v<T>)
		{
			return erase(pos, std::next(pos));
		}

		constexpr void swap(inplace_vector& other) noexcept(N == 0 ||
															(std::is_nothrow_swappable_v<T> &&
															 std::is_nothrow_move_constructible_v<T>))
			requires (!std::is_const_v<T>)
		{
			inplace_vector& small = (size() < other.size()) ? *this : other;
			inplace_vector& large = (size() < other.size()) ? other : *this;
			size_type idx = 0, small_size = small.size();
			for (; idx < small_size; ++idx) {
				using std::swap;
				swap(small[idx], large[idx]);
			}
			for (; idx < large.size(); ++idx) {
				small.unchecked_push_back(std::move(large[idx]));
			}
			large.shrink_to(small_size);
		}

		constexpr friend void swap(inplace_vector& lhs, inplace_vector& rhs) noexcept(
			N == 0 || (std::is_nothrow_swappable_v<T> && std::is_nothrow_move_constructible_v<T>)) {
			lhs.swap(rhs);
		}

		constexpr friend auto operator<=>(const inplace_vector& lhs, const inplace_vector& rhs) {
			return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
		}

		friend bool operator==(const inplace_vector& lhs, const inplace_vector& rhs) {
			if (lhs.size() != rhs.size()) return false;
			return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
		}
	};

	template<class T, size_t N, class U = T>
	constexpr typename inplace_vector<T, N>::size_type erase(inplace_vector<T, N>& c, const U& value) {
		auto it = std::remove(c.begin(), c.end(), value);
		auto r = static_cast<typename inplace_vector<T, N>::size_type>(std::distance(it, c.end()));
		c.erase(it, it.end());
		return r;
	}

	template<class T, size_t N, class Predicate>
	constexpr typename inplace_vector<T, N>::size_type erase_if(inplace_vector<T, N>& c, Predicate pred) {
		auto it = std::remove_if (c.begin(), c.end(), pred);
		auto r = static_cast<typename inplace_vector<T, N>::size_type>(std::distance(it, c.end()));
		c.erase(it, c.end());
		return r;
	}

} // namespace plg

namespace std {
	template<class T, std::size_t N>
	using inplace_vector = plg::inplace_vector<T, N>;
} // namespace std

#endif