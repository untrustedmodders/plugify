#pragma once

#include <algorithm>
#include <compare>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <memory_resource> // for polymorphic_allocator
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifndef PLUGIFY_VECTOR_EXCEPTIONS
#  if __cpp_exceptions
#    define PLUGIFY_VECTOR_EXCEPTIONS 1
#  else
#    define PLUGIFY_VECTOR_EXCEPTIONS 0
#  endif
#endif

#if PLUGIFY_VECTOR_EXCEPTIONS && (!__cpp_exceptions || !__has_include(<stdexcept>))
#  undef PLUGIFY_VECTOR_EXCEPTIONS
#  define PLUGIFY_VECTOR_EXCEPTIONS 0
#endif

#ifndef PLUGIFY_VECTOR_FALLBACK_ASSERT
#  define PLUGIFY_VECTOR_FALLBACK_ASSERT 1
#endif

#if PLUGIFY_VECTOR_FALLBACK_ASSERT && !__has_include(<cassert>)
#  undef PLUGIFY_VECTOR_FALLBACK_ASSERT
#  define PLUGIFY_VECTOR_FALLBACK_ASSERT 0
#endif

#ifndef PLUGIFY_VECTOR_FALLBACK_ABORT
#  define PLUGIFY_VECTOR_FALLBACK_ABORT 1
#endif

#if PLUGIFY_VECTOR_FALLBACK_ABORT && !__has_include(<cstdlib>)
#  undef PLUGIFY_VECTOR_FALLBACK_ABORT
#  define PLUGIFY_VECTOR_FALLBACK_ABORT 0
#endif

#ifndef PLUGIFY_VECTOR_FALLBACK_ABORT_FUNCTION
#  define PLUGIFY_VECTOR_FALLBACK_ABORT_FUNCTION [] (auto) { }
#endif

#if PLUGIFY_VECTOR_EXCEPTIONS
#  include <stdexcept>
#  define _PLUGIFY_VECTOR_ASSERT(x, str, e) do { if (!(x)) throw e(str); } while (0)
#elif PLUGIFY_VECTOR_FALLBACK_ASSERT
#  include <cassert>
#  define _PLUGIFY_VECTOR_ASSERT(x, str, ...) assert(x && str)
#elif PLUGIFY_VECTOR_FALLBACK_ABORT
#  if !PLUGIFY_VECTOR_NUMERIC_CONVERSIONS
#    include <cstdlib>
#  endif
#  define _PLUGIFY_VECTOR_ASSERT(x, ...) do { if (!(x)) { std::abort(); } } while (0)
#else
#  define _PLUGIFY_VECTOR_ASSERT(x, str, ...) do { if (!(x)) { PLUGIFY_VECTOR_FALLBACK_ABORT_FUNCTION (str); { while (true) { [] { } (); } } } } while (0)
#endif

#if defined(_MSC_VER)
#define _PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define _PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
#pragma once

namespace plg {
	namespace {
		template<typename Allocator, typename = void>
		struct is_allocator : std::false_type {};

		template<typename Allocator>
		struct is_allocator<Allocator, std::void_t<typename Allocator::value_type, decltype(std::declval<Allocator&>().allocate(std::size_t{}))>> : std::true_type {};

		template<typename Allocator>
		constexpr inline bool is_allocator_v = is_allocator<Allocator>::value;

		template<typename Iterator>
		using iterator_value_t = typename std::iterator_traits<Iterator>::value_type;

		// Contains algorithms useful for container classes.
		//
		// Synopsis:
		//
		// make_range( begin, end )
		//   Returns a range that you can use with range for loop or other std::range
		//   stuff
		// zip_transform(dst, fst, fst_end, [snd, third, rest...], n-ary op)
		//   Applies op on each element in the specified ranges, if snd, third, etc are
		//   at least as long as fst..fst_end, inserting results into dst
		// zip_foreach(fst, fst_end, [snd, third, rest...], n-ary op)
		//   Applies op on each element in the specified ranges, if snd, third, etc are
		//   at least as long as fst..fst_end
		// uninitialized_copy(src, src_end, dst)
		//   Like std::uninitialized_copy, but supports a custom allocator
		// uninitialized_move(src, src_end, dst)
		//   Like std::uninitialized_move, but supports a custom allocator
		// uninitialized_move_if_noexcept(src, src_end, dst)
		//   Like the above but with move_if_noexcept
		//
		// *_launder
		//   Like the above, but where the pointers in src..src_end are laundered

		template<std::input_or_output_iterator It, std::input_or_output_iterator It2>
		[[nodiscard]] constexpr //
				auto
				make_range(It begin, It2 end) //
				noexcept(std::is_nothrow_constructible_v<std::decay_t<It>, It&&> and
						 std::is_nothrow_constructible_v<std::decay_t<It2>, It2&&>)
		{
			using InnerIt = std::decay_t<It>;
			using InnerIt2 = std::decay_t<It2>;
			struct Range
			{
				InnerIt m_begin;
				InnerIt2 m_end;

				[[nodiscard]] constexpr InnerIt begin() const noexcept { return m_begin; }
				[[nodiscard]] constexpr InnerIt2 end() const noexcept { return m_end; }
			};
			return Range{ std::forward<It>(begin), std::forward<It2>(end) };
		}

		template<std::input_or_output_iterator OutputIt,
				 std::input_iterator FstIt,
				 typename Op,
				 std::input_iterator... RestIt>
		constexpr //
				OutputIt
				zip_transform(FstIt fst, FstIt fst_end, OutputIt dst, Op op, RestIt... rest)
		{
			for (; fst != fst_end; ++dst, ++fst, (++rest, ...)) {
				*dst = op(*fst, *rest...);
			}
			return dst;
		}

		template<std::input_iterator FstIt, typename Op, std::input_iterator... RestIt>
		constexpr //
				void
				zip_foreach(FstIt fst, FstIt fst_end, Op op, RestIt... rest)
		{
			for (; fst != fst_end; ++fst, (++rest, ...)) {
				op(*fst, *rest...);
			}
		}

		template<std::input_iterator InputIt,
				 std::input_or_output_iterator OutputIt,
				 typename Allocator = std::allocator<iterator_value_t<OutputIt>>>
		constexpr //
				OutputIt
				uninitialized_copy(InputIt src, InputIt src_end, OutputIt dst, Allocator alloc)
		{
			for (; src != src_end; ++src, ++dst) {
				std::allocator_traits<Allocator>::construct(alloc, dst, *src);
			}
			return dst;
		}

		template<std::input_iterator InputIt,
				 std::input_or_output_iterator OutputIt,
				 typename Allocator = std::allocator<iterator_value_t<OutputIt>>>
		constexpr //
				OutputIt
				uninitialized_move(InputIt src, InputIt src_end, OutputIt dst, Allocator alloc)
		{
			for (; src != src_end; ++src, ++dst) {
				std::allocator_traits<Allocator>::construct(alloc, dst, std::move(*src));
			}
			return dst;
		}

		template<std::input_iterator InputIt,
				 std::input_or_output_iterator OutputIt,
				 typename Allocator = std::allocator<iterator_value_t<OutputIt>>>
		constexpr //
				OutputIt
				uninitialized_move_if_noexcept(InputIt src, InputIt src_end, OutputIt dst, Allocator alloc)
		{
			for (; src != src_end; ++src, ++dst) {
				std::allocator_traits<Allocator>::construct(alloc, dst, std::move_if_noexcept(*src));
			}
			return dst;
		}

		template<std::input_iterator InputIt,
				 std::input_or_output_iterator OutputIt,
				 typename Allocator = std::allocator<iterator_value_t<OutputIt>>>
		constexpr //
				OutputIt
				uninitialized_copy_launder(InputIt src, InputIt src_end, OutputIt dst, Allocator alloc)
		{
			for (; src != src_end; ++src, ++dst) {
				std::allocator_traits<Allocator>::construct(alloc, dst, *std::launder(src));
			}
			return dst;
		}

		template<std::input_iterator InputIt,
				 std::input_or_output_iterator OutputIt,
				 typename Allocator = std::allocator<iterator_value_t<OutputIt>>>
		constexpr //
				OutputIt
				uninitialized_move_launder(InputIt src, InputIt src_end, OutputIt dst, Allocator alloc)
		{
			for (; src != src_end; ++src, ++dst) {
				std::allocator_traits<Allocator>::construct(alloc, dst, std::move(*std::launder(src)));
			}
			return dst;
		}

		template<std::input_iterator InputIt,
				 std::input_or_output_iterator OutputIt,
				 typename Allocator = std::allocator<iterator_value_t<OutputIt>>>
		constexpr //
				OutputIt
				uninitialized_move_if_noexcept_launder(InputIt src,
													   InputIt src_end,
													   OutputIt dst,
													   Allocator alloc)
		{
			for (; src != src_end; ++src, ++dst) {
				std::allocator_traits<Allocator>::construct(
						alloc, dst, std::move_if_noexcept(*std::launder(src)));
			}
			return dst;
		}

		template<std::input_iterator InputIt,
				 std::input_or_output_iterator OutputIt,
				 typename Allocator = std::allocator<iterator_value_t<OutputIt>>>
		constexpr //
				OutputIt
				move_if_noexcept_launder_backward(InputIt src, InputIt src_end, OutputIt dst_end)
		{
			for (; src != src_end; --src_end, --dst_end) {
				--src_end;
				--dst_end;
				*dst_end = std::move_if_noexcept(*std::launder(src_end));
			}
			return dst_end;
		}

		template<std::input_iterator InputIt,
				 std::input_or_output_iterator OutputIt,
				 typename Allocator = std::allocator<iterator_value_t<OutputIt>>>
		constexpr //
				OutputIt
				uninitialized_move_if_noexcept_launder_backward(InputIt src,
																InputIt src_end,
																OutputIt dst_end,
																Allocator alloc)
		{
			for (; src != src_end; --src_end, --dst_end) {
				--src_end;
				--dst_end;
				std::allocator_traits<Allocator>::construct(
						alloc, dst_end, std::move_if_noexcept(*std::launder(src_end)));
			}
			return dst_end;
		}

	} // namespace

	template<typename T, typename Allocator>
	struct vector_base {
		//////////////////
		// Member types //
		//////////////////

	private:
		// Purely to make notation easier
		using allocator_traits = std::allocator_traits<Allocator>;

	public:
		using value_type = T;
		using allocator_type = Allocator;
		using size_type = typename allocator_traits::size_type;
		using difference_type = typename allocator_traits::difference_type;
		using reference = T&;
		using const_reference = const T&;
		using pointer = typename allocator_traits::pointer;
		using const_pointer = typename allocator_traits::const_pointer;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using reverse_const_iterator = std::reverse_iterator<const_iterator>;
		using comparison_type = std::conditional_t<std::three_way_comparable<T>,
												   decltype(std::declval<T>() <=> std::declval<T>()),
												   std::weak_ordering>;

		/////////////////
		// Data layout //
		/////////////////
	private:
		pointer _begin;
		pointer _end;
		pointer _realend;
		_PLUGIFY_VECTOR_NO_UNIQUE_ADDRESS
		allocator_type _allocator;

	public:
		//////////////////
		// Constructors //
		//////////////////

		constexpr       //
				vector_base() //
				noexcept(std::is_nothrow_default_constructible<allocator_type>::value) //
			requires(is_allocator_v<Allocator>)
			: _begin(nullptr),
			  _end(nullptr),
			  _realend(nullptr),
			  _allocator()
		{}

		constexpr explicit                    //
				vector_base(const allocator_type& alloc) //
				noexcept  //
			requires(is_allocator_v<Allocator>)
			: _begin(nullptr),
			  _end(nullptr),
			  _realend(nullptr),
			  _allocator(alloc)
		{}

		constexpr //
				vector_base(size_type count, const T& value, const allocator_type& alloc = allocator_type()) //
			requires(is_allocator_v<Allocator>)
			: _allocator(alloc)
		{
			allocate(count, _allocator);
			for (auto& elem : make_range(_begin, _realend)) {
				allocator_traits::construct(_allocator, std::launder(&elem), value);
			}
			_end = _realend;
		}

		constexpr explicit //
				vector_base(size_type count, const allocator_type& alloc = allocator_type()) //
			requires(is_allocator_v<Allocator>)
			: _allocator(alloc)
		{
			allocate(count, _allocator);
			for (auto& elem : make_range(_begin, _realend)) {
				allocator_traits::construct(_allocator, std::launder(&elem));
			}
			_end = _realend;
		}

		// Looser overload that allows any input iterator
		template<std::input_iterator InputIt>
		constexpr //
				vector_base(InputIt first, InputIt last, const allocator_type& alloc = allocator_type()) //
			requires(is_allocator_v<Allocator>)
			: _begin(nullptr),
			  _end(nullptr),
			  _realend(nullptr),
			  _allocator(alloc)
		{
			for (const auto& elem : make_range(first, last)) {
				push_back(elem);
			}
		}

		// Tighter overload to reserve up front
		template<std::random_access_iterator RandomAccessIt>
		constexpr //
				vector_base(RandomAccessIt first, RandomAccessIt last, const allocator_type& alloc = allocator_type()) //
			requires(is_allocator_v<Allocator>)
			: _allocator(alloc)
		{
			if (last - first > 0) {
				allocate(last - first, _allocator);
				_end = uninitialized_copy(first, last, _begin, _allocator);
			} else {
				_begin = _end = _realend = nullptr;
			}
		}

		/////////////////////////////////////////////////////////
		// Special member functions (and similar constructors) //
		/////////////////////////////////////////////////////////

		constexpr //
				vector_base(const vector_base& other)
			: vector_base(other._begin,
						  other._end,
						  allocator_traits::select_on_container_copy_construction(other._allocator))
		{}

		constexpr //
				vector_base(const vector_base& other, const allocator_type& alloc) //
			requires(is_allocator_v<Allocator>)
			: vector_base(other._begin, other._end, alloc)
		{}

		constexpr vector_base(std::initializer_list<T> il, const allocator_type& alloc = allocator_type()) //
			requires(is_allocator_v<Allocator>)
			: vector_base(il.begin(), il.end(), alloc)
		{}

		constexpr                          //
				vector_base(vector_base&& other) //
				noexcept
			: _begin(other._begin),
			  _end(other._end),
			  _realend(other._realend),
			  _allocator(std::move(other._allocator))
		{
			other._begin = other._end = other._realend = nullptr;
		}

		constexpr                                                  //
				vector_base(vector_base&& other, const allocator_type& alloc) // //
			requires(is_allocator_v<Allocator>)
			: _allocator(alloc)
		{
			if (_allocator != other._allocator) {
				allocate(other.size());
				uninitialized_move(other._begin, other._end, _begin, _allocator);
				_end = _realend;
			} else {
				_begin = other._begin;
				_end = other._end;
				_realend = other._realend;
				other._begin = other._end = other._realend = nullptr;
			}
		}

		constexpr //
				vector_base&
				operator=(const vector_base& other)
		{
			if (this == &other)
				return *this;

			if constexpr (allocator_traits::propagate_on_container_copy_assignment::value) {
				if (not allocator_traits::is_always_equal::value and _allocator != other._allocator) {
					deallocate();
				}
				allocate_empty(other.size());
				_allocator = other._allocator;
			}

			// we require realloc, so we construct into fresh array directly
			if (other.size() > capacity()) {
				auto tmp = allocate_tmp(other.size(), _allocator);
				try {
					uninitialized_copy(other._begin, other._end, tmp, _allocator);
				} catch (...) {
					allocator_traits::deallocate(_allocator, tmp, other.size());
					throw;
				}
				allocator_traits::deallocate(_allocator, _begin, capacity());
				_begin = tmp;
				_end = _realend = tmp + other.size();
				return *this;
			}

			// destroy excess
			while (other.size() < size()) {
				pop_back();
			}

			// copy-assign onto existing elements
			auto tmp = other._begin;
			for (auto& elem : *this) {
				elem = *std::launder(tmp);
				++tmp;
			}

			// copy-construct new elements
			while (tmp != other._end) {
				push_back(*std::launder(tmp));
				++tmp;
			}
			return *this;
		}

		constexpr //
				vector_base&
				operator=(vector_base&& other) //
				noexcept(allocator_traits::propagate_on_container_move_assignment::value ||
						 allocator_traits::is_always_equal::value)
		{
			if (this == &other)
				return *this;

			if constexpr (allocator_traits::propagate_on_container_move_assignment::value) {
				if (not allocator_traits::is_always_equal::value and _allocator != other._allocator) {
					_allocator = other._allocator;
				}
				deallocate();
				_begin = other._begin;
				_end = other._end;
				_realend = other._realend;
				other._begin = other._end = other._realend = nullptr;
			} else {
				if (not allocator_traits::is_always_equal::value and _allocator != other._allocator) {
					// We must move-assign elements :(
					if (other.size() > capacity()) {
						// We must realloc, so directly move into new buffer
						auto tmp = allocate_tmp(other.size(), _allocator);
						try {
							uninitialized_move(other._begin, other._end, tmp, _allocator);
							deallocate();
							_begin = tmp;
							_realend = _end = tmp + other.size();
						} catch (...) {
							allocator_traits::deallocate(_allocator, tmp, other.size());
							throw;
						}
					} else {
						// destroy excess
						while (other.size() < size()) {
							pop_back();
						}

						// move-assign onto existing elements
						auto tmp = other._begin;
						for (auto& elem : *this) {
							elem = std::move(*std::launder(tmp));
							++tmp;
						}

						// move-construct new elements
						while (tmp != other._end) {
							push_back(std::move(*std::launder(tmp)));
							++tmp;
						}
					}
				} else {
					deallocate();
					_begin = other._begin;
					_end = other._end;
					_realend = other._realend;
					other._begin = other._end = other._realend = nullptr;
				}
			}
			return *this;
		}

		constexpr //
				vector_base&
				operator=(std::initializer_list<T> ilist)
		{
			assign(ilist.begin(), ilist.end());
			return *this;
		}

		constexpr //
				void
				swap(vector_base& other) //
				noexcept(allocator_traits::propagate_on_container_swap::value ||
						 allocator_traits::is_always_equal::value)
		{
			if constexpr (allocator_traits::propagate_on_container_swap::value) {
				using std::swap;
				swap(_allocator, other._allocator);
			}
			// We're allowed to UB if m_alloc != other.m_alloc and propagate is false
			// This is cause swap must be constant time, if propagate is false and allocs are not equal
			// we would be forced to copy / move (and thus not be constant time anymore)
			std::swap(_begin, other._begin);
			std::swap(_end, other._end);
			std::swap(_realend, other._realend);
		}

		friend //
				void
				swap(vector_base& a, vector_base& b) //
				noexcept(allocator_traits::propagate_on_container_swap::value ||
						 allocator_traits::is_always_equal::value)
		{
			a.swap(b);
		}

		constexpr ~vector_base() { deallocate(); }

	private:
		constexpr //
				void
				check_range(size_type n) //
				const
		{
			if (n >= size()) {
				// TODO: do fancier formatting when I implement constexpr string (?)
				throw std::out_of_range("Bounds check failed.");
			}
		}

	public:
		[[nodiscard]] constexpr //
				reference
				at(size_type pos)
		{
			check_range(pos);
			return *this[pos];
		}
		[[nodiscard]] constexpr //
				const_reference
				at(size_type pos) //
				const
		{
			check_range(pos);
			return *this[pos];
		}

		[[nodiscard]] constexpr //
				reference
				operator[](size_type pos) //
				noexcept
		{
			return *std::launder(_begin + pos);
		}
		[[nodiscard]] constexpr //
				const_reference
				operator[](size_type pos) //
				const noexcept
		{
			return *std::launder(_begin + pos);
		}

		/////////////
		// Getters //
		/////////////

		[[nodiscard]] constexpr /********/ pointer data() /************/ noexcept { return _begin; }
		[[nodiscard]] constexpr /**/ const_pointer data() /******/ const noexcept { return _begin; }
		[[nodiscard]] constexpr /******/ allocator_type get_allocator() const noexcept { return _allocator; }

		[[nodiscard]] constexpr /***/ reference front() /********/ noexcept { return *_begin; }
		[[nodiscard]] constexpr const_reference front() /**/ const noexcept { return *_begin; }
		[[nodiscard]] constexpr /***/ reference back() /*********/ noexcept { return *(_end - 1); }
		[[nodiscard]] constexpr const_reference back() /***/ const noexcept { return *(_end - 1); }

		[[nodiscard]] constexpr /***/ iterator begin() /*********/ noexcept { return _begin; }
		[[nodiscard]] constexpr const_iterator begin() /***/ const noexcept { return _begin; }
		[[nodiscard]] constexpr /***/ iterator end() /***********/ noexcept { return _end; }
		[[nodiscard]] constexpr const_iterator end() /*****/ const noexcept { return _end; }
		[[nodiscard]] constexpr const_iterator cbegin() /**/ const noexcept { return _begin; }
		[[nodiscard]] constexpr const_iterator cend() /****/ const noexcept { return _end; }

		[[nodiscard]] constexpr /***/ reverse_iterator rbegin() /*********/ noexcept { return _end; }
		[[nodiscard]] constexpr reverse_const_iterator rbegin() /***/ const noexcept { return _end; }
		[[nodiscard]] constexpr /***/ reverse_iterator rend() /***********/ noexcept { return _begin; }
		[[nodiscard]] constexpr reverse_const_iterator rend() /*****/ const noexcept { return _begin; }
		[[nodiscard]] constexpr reverse_const_iterator crbegin() /**/ const noexcept { return _end; }
		[[nodiscard]] constexpr reverse_const_iterator crend() /****/ const noexcept { return _begin; }

		[[nodiscard]] constexpr size_type size() /******/ const noexcept { return _end - _begin; }
		[[nodiscard]] constexpr size_type capacity() /**/ const noexcept { return _realend - _begin; }
		[[nodiscard]] constexpr bool empty() /**********/ const noexcept { return size() == 0; }
		[[nodiscard]] constexpr //
				size_type
				max_size() //
				const
		{
			const size_type diffmax = std::numeric_limits<difference_type>::max() / sizeof(T);
			const size_type allocmax = allocator_traits::max_size(_allocator);
			return std::min(diffmax, allocmax);
		}

		////////////////////
		// Size modifiers //
		////////////////////

		constexpr //
				void
				reserve(size_type new_cap)
		{
			if (new_cap > capacity()) {
				auto tmp = allocate_tmp(new_cap, _allocator);
				try {
					auto end = uninitialized_move_if_noexcept_launder(_begin, _end, tmp, _allocator);
					deallocate();
					_begin = tmp;
					_end = end;
					_realend = tmp + new_cap;
				} catch (...) {
					allocator_traits::deallocate(tmp);
					throw;
				}
			}
		}

		constexpr //
				void
				shrink_to_fit()
		{
			auto oldsize = size();
			if (oldsize < capacity()) {
				auto tmp = allocate_tmp(oldsize, _allocator);
				try {
					auto end = uninitialized_move_launder(_begin, _end, tmp, _allocator);
					deallocate();
					_begin = tmp;
					_end = end;
					_realend = tmp + oldsize;
				} catch (...) {
					allocator_traits::deallocate(tmp);
					throw;
				}
			}
		}

		constexpr //
				void
				resize(size_type count)
		{
			if (count > capacity()) {
				auto tmp = allocate_tmp(count, _allocator);
				try {
					auto end = uninitialized_move_if_noexcept_launder(_begin, _end, tmp, _allocator);
					for (; end < tmp + count; ++end) {
						allocator_traits::construct(_allocator, end);
					}
					deallocate();
					_begin = tmp;
					_end = end;
					_realend = tmp + count;
				} catch (...) {
					allocator_traits::deallocate(tmp);
					throw;
				}
			} else if (count > size()) {
				while (size() > count) {
					emplace_back();
				}
			} else {
				while (size() > count) {
					pop_back();
				}
			}
		}

		constexpr //
				void
				resize(size_type count, const value_type& value)
		{
			if (count > capacity()) {
				auto tmp = allocate_tmp(count, _allocator);
				try {
					// We construct new elements first in case value is part of vector_base
					auto end = tmp + size();
					for (; end < tmp + count; ++end) {
						allocator_traits::construct(_allocator, end, value);
					}
					end = uninitialized_move_if_noexcept_launder(_begin, _end, tmp, _allocator);
					deallocate();
					_begin = tmp;
					_end = end;
					_realend = tmp + count;
				} catch (...) {
					allocator_traits::deallocate(tmp);
					throw;
				}
			} else if (count > size()) {
				while (size() > count) {
					emplace_back(value);
				}
			} else {
				while (size() > count) {
					pop_back();
				}
			}
		}

		constexpr //
				void
				clear() //
				noexcept
		{
			while (not empty()) {
				pop_back();
			}
		}

		/////////////////////////
		// Assigning modifiers //
		/////////////////////////

		/*constexpr void assign(size_type count, const T& value)
		{
			// TODO: Finish
		}*/

		template< class InputIt >
		constexpr void assign(InputIt first, InputIt last)
		{
			assign(std::initializer_list<T>(first, last));
		}

		constexpr void assign(std::initializer_list<T> ilist)
		{
			if (ilist.size() > capacity()) {
				// We must realloc, so directly move into new buffer
				auto tmp = allocate_tmp(ilist.size(), _allocator);
				try {
					uninitialized_move(ilist.begin(), ilist.end(), tmp, _allocator);
					deallocate();
					_begin = tmp;
					_realend = _end = tmp + ilist.size();
				} catch (...) {
					allocator_traits::deallocate(_allocator, tmp, ilist.size());
					throw;
				}
			} else {
				// destroy excess
				while (ilist.size() < size()) {
					pop_back();
				}

				// copy-assign onto existing elements
				auto tmp = ilist.begin();
				for (auto& elem : *this) {
					elem = *tmp;
					++tmp;
				}

				// copy-construct new elements
				while (tmp != ilist.end()) {
					push_back(*tmp);
					++tmp;
				}
			}
		}

		/////////////////////////
		// Insertion modifiers //
		/////////////////////////

		// Strong exception guarantee
		template<typename... Args>
		constexpr //
				void
				emplace_back(Args&&... args)
		{
			if (_end < _realend) {
				allocator_traits::construct(_allocator, std::launder(_end), std::forward<Args>(args)...);
				++_end;
			}

			// Ensure we've fully prepared a tmp buffer before deallocating m_begin
			auto oldsize = size();
			auto newcap = size() * 2 + 1;
			auto tmp = allocate_tmp(newcap, _allocator);
			try {
				// construct new value into tmp, we should do this first in case input is part of the
				// vector_base
				allocator_traits::construct(_allocator, tmp + oldsize, std::forward<Args>(args)...);
				// move existing values if noexcept, else copy
				uninitialized_move_if_noexcept_launder(_begin, _end, tmp, _allocator);
			} catch (...) {
				allocator_traits::deallocate(_allocator, tmp, newcap);
				throw;
			}
			// buffer is ready, do the swap
			allocator_traits::deallocate(_allocator, _begin, capacity());
			_begin = tmp;
			_end = tmp + oldsize + 1;
			_realend = tmp + newcap;
		}

		// Strong exception guarantee
		constexpr void push_back(const T& v) { emplace_back(v); }
		// Strong exception guarantee
		constexpr void push_back(T&& v) { emplace_back(std::move(v)); }

		// Conditionally strong exception guarantee
		// as long as value_type is nothrow assignable and constructible either by move or copy.
		template<typename... Args>
		constexpr //
				pointer
				emplace(const_pointer pos, Args&&... args)
		{
			if (pos == _end) {
				emplace_back(args...);
				return _end - 1;
			}

			if (_end == _realend) {
				// We need to realloc
				auto index = pos - _begin;
				auto oldsize = size();
				auto newcap = size() * 2 + 1;
				auto tmp = allocate_tmp(newcap, _allocator);
				try {
					// construct new value into tmp, we should do this first in case input is part of the
					// vector_base
					allocator_traits::construct(_allocator, tmp + index, std::forward<T>(args)...);
					// move existing values if noexcept, else copy
					uninitialized_move_if_noexcept_launder(_begin, pos, tmp, _allocator);
					uninitialized_move_if_noexcept_launder(pos, _end, tmp + index + 1, _allocator);
				} catch (...) {
					allocator_traits::deallocate(_allocator, tmp, newcap);
					throw;
				}
				// buffer is ready, do the swap
				allocator_traits::deallocate(_allocator, _begin, capacity());
				_begin = tmp;
				_end = tmp + oldsize + 1;
				_realend = tmp + newcap;
				return _begin + index;
			}

			// No realloc needed
			// We're allowed to UB if the move / copy constructor / assignment throws
			// ... unfortunately we can't shift the elements first, THEN construct
			// because if the constructor throws we aren't supposed to UB
			// So we start by constructing the element into a temporary that we move into place later.
			auto tmp = T(std::forward<T>(args)...);
			// After this point, everything is either allowed to UB or is noexcept :)

			// Shift elements back
			uninitialized_move_if_noexcept_launder_backward(_end - 1, _end, _end + 1, _allocator);
			move_if_noexcept_launder_backward(pos, _end - 1, _end);
			// Now move the tmp var into place
			*pos = std::move_if_noexcept(tmp);
			return pos;
		}

		constexpr //
				iterator
				insert(const_iterator pos, const T& value)
		{
			insert(pos, 1, value);
		}

		constexpr //
				iterator
				insert(const_iterator pos, T&& value)
		{
			if (pos == _end) {
				emplace_back(value);
				return _end - 1;
			}

			if (_end + 1 < _realend) {
				// We need to realloc
				auto index = pos - _begin;
				auto oldsize = size();
				auto newcap = size() * 2 + 1;
				auto tmp = allocate_tmp(newcap, _allocator);
				try {
					// construct new values into tmp, we should do this first in case input is part of the
					// vector_base
					allocator_traits::construct(_allocator, tmp + index, std::move(value));
					// move existing values if noexcept, else copy
					uninitialized_move_if_noexcept_launder(_begin, pos, tmp, _allocator);
					uninitialized_move_if_noexcept_launder(pos, _end, tmp + index + 1, _allocator);
				} catch (...) {
					allocator_traits::deallocate(_allocator, tmp, newcap);
					throw;
				}
				// buffer is ready, do the swap
				allocator_traits::deallocate(_allocator, _begin, capacity());
				_begin = tmp;
				_end = tmp + oldsize + 1;
				_realend = tmp + newcap;
				return _begin + index;
			}

			// No realloc needed
			// Shift elements back
			uninitialized_move_if_noexcept_launder_backward(_end - 1, _end, _end + 1, _allocator);
			move_if_noexcept_launder_backward(pos, _end - 1, _end);
			// Now move value into place
			pos = std::move(value);
			return pos;
		}

		constexpr //
				iterator
				insert(const_iterator pos, size_type count, const T& value)
		{
			if (count != 0) {
				if (pos == _end) {
					emplace_back(value);
					return _end - 1;
				}

				if (_end + count < _realend) {
					// We need to realloc
					auto index = pos - _begin;
					auto oldsize = size();
					auto newcap = size() * 2 + count;
					auto tmp = allocate_tmp(newcap, _allocator);
					try {
						// construct new values into tmp, we should do this first in case input is part of the
						// vector_base
						for (auto it = tmp + index; it < tmp + index + count; ++it) {
							allocator_traits::construct(_allocator, it, value);
						}
						// move existing values if noexcept, else copy
						uninitialized_move_if_noexcept_launder(_begin, pos, tmp, _allocator);
						uninitialized_move_if_noexcept_launder(pos, _end, tmp + index + count, _allocator);
					} catch (...) {
						allocator_traits::deallocate(_allocator, tmp, newcap);
						throw;
					}
					// buffer is ready, do the swap
					allocator_traits::deallocate(_allocator, _begin, capacity());
					_begin = tmp;
					_end = tmp + oldsize + count;
					_realend = tmp + newcap;
					return _begin + index;
				}

				// No realloc needed
				// Shift elements back
				uninitialized_move_if_noexcept_launder_backward(_end - count, _end, _end + count, _allocator);
				move_if_noexcept_launder_backward(pos, _end - count, _end);
				// Now copy the value into place repeatedly
				std::fill(pos, pos + count, value);
				return pos;
			}
		}

		// Not quite the same as LegacyInputIterator,
		// but this way is easier and shouldn't break any existing code anyway.
		template<std::input_iterator InputIt>
		constexpr //
				iterator
				insert(const_iterator pos, InputIt first, InputIt last)
		{
			// Since input iterator is single pass, we will just insert one at a time the dumb way.
			// TODO: copy first to a vector then call the random_access_iterator overload.

			// Convert iterator to an index first to handle reallocation case
			auto index = std::distance(_begin, pos);
			while (first != last) {
				insert(_begin + index, *first);
				++index;
				++first;
			}
			return _begin + index;
		}

		constexpr //
				iterator
				insert(const_iterator pos, std::initializer_list<T> ilist)
		{
			return insert(pos, ilist.begin(), ilist.end());
		}

		///////////////////////
		// Removal modifiers //
		///////////////////////

		constexpr //
				void
				pop_back() //
		{
			allocator_traits::destroy(_allocator, std::launder(_end - 1));
			--_end;
		}

		constexpr //
				iterator
				erase(const_iterator pos)
		{
			return erase(pos, pos + 1);
		}

		constexpr //
				iterator
				erase(const_iterator first, const_iterator last)
		{
			move_if_noexcept_launder(last, _end, first, _allocator);
			for (size_t i = 0; i < last - first; ++i) {
				pop_back();
			}
			return first;
		}

		//////////////////////////
		// Comparison operators //
		//////////////////////////

		[[nodiscard]] constexpr //
				bool
				operator==(const vector_base& other)                 //
				const noexcept(noexcept(*begin() == *other.begin())) //
			requires std::equality_comparable<T>
		{
			return std::equal(begin(), end(), other.begin(), other.end());
		}

		[[nodiscard]] constexpr //
				comparison_type
				operator<=>(const vector_base& other)                //
				const noexcept(noexcept(*begin() == *other.begin())) //
			requires std::three_way_comparable<T> ||             //
					 requires(const T& elem)
		{
			elem < elem;
		} //
		{
			if constexpr (std::three_way_comparable<T>) {
				return std::lexicographical_compare_three_way(_begin, _end, other._begin, other._end);
			} else {
				return std::lexicographical_compare_three_way(
						_begin, _end, other._begin, other._end, [](const auto& a, const auto& b) {
							return a < b ? std::weak_ordering::less :
								   b < a ? std::weak_ordering::greater :
										 std::weak_ordering::equivalent;
						});
			}
		}

		/////////////////////////////////////////
		// Allocation / deallocation utilities //
		/////////////////////////////////////////

	private:
		constexpr //
				void
				allocate(size_type capacity, Allocator& alloc)
		{
			try {
				_begin = allocator_traits::allocate(alloc, capacity);
				_realend = _begin + capacity;
			} catch (...) {
				if (capacity > max_size()) {
					throw std::length_error("Tried to allocate too many elements.");
				} else {
					throw;
				}
			}
		}

		constexpr //
				pointer
				allocate_tmp(size_type capacity, Allocator& alloc)
		{
			try {
				return allocator_traits::allocate(alloc, capacity, _begin);
			} catch (...) {
				if (capacity > max_size()) {
					throw std::length_error("Tried to allocate too many elements.");
				} else {
					throw;
				}
			}
		}

		constexpr //
				void
				deallocate() //
				noexcept
		{
			clear();
			if (_begin)
				allocator_traits::deallocate(_allocator, _begin, capacity());
		}
	};

	template<typename T, typename Alloc, typename U>
	constexpr //
			typename vector_base<T, Alloc>::size_type
			erase(vector_base<T, Alloc>& c, const U& value)
	{
		auto it = std::remove(c.begin(), c.end(), value);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	template<typename T, typename Alloc, typename Pred>
	constexpr //
			typename vector_base<T, Alloc>::size_type
			erase_if(vector_base<T, Alloc>& c, Pred pred)
	{
		auto it = std::remove_if(c.begin(), c.end(), pred);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	template<typename T, typename Allocator = std::allocator<T>>
	using vector = vector_base<T, Allocator>;

	namespace pmr {
		template<typename T>
		using vector = ::plg::vector_base<T, std::pmr::polymorphic_allocator<T>>;

	} // namespace pmr

} // namespace plg