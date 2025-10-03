#pragma once

namespace plg {
	template <class, class Alloc, class... Args>
	struct has_construct_impl : std::false_type {};

	template <class Alloc, class... Args>
	struct has_construct_impl<
		decltype((void) std::declval<Alloc>().construct(std::declval<Args>()...)),
		Alloc,
		Args...> : std::true_type {};

	template <class Alloc, class... Args>
	struct has_construct : has_construct_impl<void, Alloc, Args...> {};

	// __has_destroy
	template <class Alloc, class Pointer, class = void>
	struct has_destroy : std::false_type {};

	template <class Alloc, class Pointer>
	struct has_destroy<Alloc, Pointer, decltype((void) std::declval<Alloc>().destroy(std::declval<Pointer>()))>
		: std::true_type {};

	template <class Alloc, class Type>
	struct allocator_has_trivial_move_construct : std::negation<has_construct<Alloc, Type*, Type&&>> {};

	template <class Type>
	struct allocator_has_trivial_move_construct<allocator<Type>, Type> : std::true_type {};

	template <class Alloc, class T>
	struct allocator_has_trivial_destroy : std::negation<has_destroy<Alloc, T*>> {};

	template <class T, class U>
	struct allocator_has_trivial_destroy<allocator<T>, U> : std::true_type {};

	template <class Alloc, class Type>
	struct allocator_has_trivial_copy_construct
		: std::negation<has_construct<Alloc, Type*, const Type&>> {};

	template <class Type>
	struct allocator_has_trivial_copy_construct<allocator<Type>, Type> : std::true_type {};

	// Destroy all elements in [__first, __last) from left to right using allocator destruction.
	template <class Alloc, class Iter, class Sent>
	void allocator_destroy(Alloc& alloc, Iter first, Sent last) {
		for (; first != last; ++first)
			std::allocator_traits<Alloc>::destroy(alloc, std::to_address(first));
	}

	template <class Alloc, class Iter>
	class AllocatorDestroyRangeReverse {
	public:
		AllocatorDestroyRangeReverse(Alloc& alloc, Iter& first, Iter& last)
			: alloc_(alloc), first_(first), last_(last) {}

		void operator()() const {
			allocator_destroy(alloc_, std::reverse_iterator<Iter>(last_), std::reverse_iterator<Iter>(first_));
		}

	private:
		Alloc& alloc_;
		Iter& first_;
		Iter& last_;
	};

	// Copy-construct [first1, last1) in [first2, first2 + N), where N is
	// distance(first1, last1).
	//
	// The caller has to ensure that first2 can hold at least N uninitialized elements. If an
	// exception is thrown the already copied elements are destroyed in reverse order of their
	// construction.
	template <class Alloc, class Iter1, class Sent1, class Iter2>
	Iter2 uninitialized_allocator_copy_impl(
		Alloc& alloc,
		Iter1 first1,
		Sent1 last1,
		Iter2 first2
	) {
		auto destruct_first = first2;
		auto guard = make_exception_guard(
			AllocatorDestroyRangeReverse<Alloc, Iter2>(alloc, destruct_first, first2)
		);
		while (first1 != last1) {
			std::allocator_traits<Alloc>::construct(alloc, std::to_address(first2), *first1);
			++first1;
			++first2;
		}
		guard.complete();
		return first2;
	}

	template <
		class Alloc,
		class In,
		class RawTypeIn = std::remove_const_t<In>,
		class Out>
	// using RawTypeIn because of the allocator<T const> extension
	requires (std::is_trivially_copy_constructible_v<RawTypeIn> &&
		std::is_trivially_copy_assignable_v<RawTypeIn> &&
		std::is_same_v<std::remove_const_t<In>, std::remove_const_t<Out>> &&
		allocator_has_trivial_copy_construct<Alloc, RawTypeIn>::value)
	Out* uninitialized_allocator_copy_impl(Alloc&, In* first1, In* last1, Out* first2) {
		return std::copy(first1, last1, const_cast<RawTypeIn*>(first2));
	}

	template <class Alloc, class Iter1, class Sent1, class Iter2>
	Iter2 uninitialized_allocator_copy(Alloc& alloc, Iter1 first1, Sent1 last1, Iter2 first2) {
		return uninitialized_allocator_copy_impl(alloc, std::move(first1), std::move(last1), std::move(first2));
	}

	// __uninitialized_allocator_relocate relocates the objects in [__first, __last) into
	// __result.
	// Relocation means that the objects in [__first, __last) are placed into __result as-if by
	// move-construct and destroy, except that the move constructor and destructor may never be
	// called if they are known to be equivalent to a memcpy.
	//
	// Preconditions:  __result doesn't contain any objects and [__first, __last) contains
	// objects Postconditions: __result contains the objects from [__first, __last) and
	//                 [__first, __last) doesn't contain any objects
	//
	// The strong exception guarantee is provided if any of the following are true:
	// - is_nothrow_move_constructible<T>
	// - is_copy_constructible<T>
	// - is_trivially_relocatable<T>
	template <class Alloc, class T>
	constexpr void uninitialized_allocator_relocate(Alloc& alloc, T* first, T* last, T* result) {
		if (std::is_constant_evaluated() ||
			!is_trivially_relocatable<T>::value ||
			!allocator_has_trivial_move_construct<Alloc, T>::value ||
			!allocator_has_trivial_destroy<Alloc, T>::value
		) {
			auto destruct_first = result;
			auto guard = make_exception_guard(
				AllocatorDestroyRangeReverse<Alloc, T*>(alloc, destruct_first, result)
			);
			auto iter = first;
			while (iter != last) {
				std::allocator_traits<Alloc>::construct(
					alloc,
					result,
#if PLUGIFY_HAS_EXCEPTIONS
					std::move_if_noexcept(*iter)
#else
					std::move(*iter)
#endif // PLUGIFY_HAS_EXCEPTIONS
				);
				++iter;
				++result;
			}
			guard.complete();
			allocator_destroy(alloc, first, last);
		} else {
			// Casting to void* to suppress clang complaining that this is technically UB.
			std::memcpy(static_cast<void*>(result), first, sizeof(T) * static_cast<size_t>(last - first));
		}
	}
};  // namespace plg
