#pragma once

#include "plg/vector.hpp"
#include "plg/inplace_vector.hpp"
#include "plg/variant.hpp"

namespace plg {
	// hybrid container that uses std::inplace_vector for stack storage up to N elements, then switches to std::vector for larger sizes.
	template<typename T, std::size_t N>  requires (N > 0)
	class hybrid_vector {
	private:
	    variant<inplace_vector<T, N>, vector<T>> storage_;

	    // Check if currently using stack storage
	    [[nodiscard]] constexpr bool is_small() const noexcept {
	        return storage_.index() == 0;
	    }

	    // Promote from stack to heap storage
	    constexpr void promote_to_heap() {
	        if (is_small()) {
	            auto& small_vec = plg::get<0>(storage_);
	            vector<T> heap_vec(
	                std::make_move_iterator(small_vec.begin()),
	                std::make_move_iterator(small_vec.end())
	            );
	            storage_.template emplace<1>(std::move(heap_vec));
	        }
	    }

	public:
	    using value_type = T;
	    using size_type = std::size_t;
	    using reference = T&;
	    using const_reference = const T&;

	    // Constructors
	    constexpr hybrid_vector() noexcept
	        : storage_(in_place_index<0>) {}

	    constexpr explicit hybrid_vector(size_type count)
	        : storage_(count <= N ?
	            variant<inplace_vector<T, N>, vector<T>>(
	                in_place_index<0>, count) :
	            variant<inplace_vector<T, N>, vector<T>>(
	                in_place_index<1>, count)) {}

	    constexpr hybrid_vector(size_type count, const T& value)
	        : storage_(count <= N ?
	            variant<inplace_vector<T, N>, vector<T>>(
	                in_place_index<0>, count, value) :
	            variant<inplace_vector<T, N>, vector<T>>(
	                in_place_index<1>, count, value)) {}

	    template<std::input_iterator It>
	    constexpr hybrid_vector(It first, It last) {
	        if constexpr (std::random_access_iterator<It>) {
	            auto count = std::distance(first, last);
	            if (count <= static_cast<std::ptrdiff_t>(N)) {
	                storage_.template emplace<0>(first, last);
	            } else {
	                storage_.template emplace<1>(first, last);
	            }
	        } else {
	            storage_.template emplace<0>();
	            for (; first != last; ++first) {
	                push_back(*first);
	            }
	        }
	    }

	    constexpr hybrid_vector(std::initializer_list<T> init)
	        : hybrid_vector(init.begin(), init.end()) {}

	    // Size operations using C++23 explicit object parameter (deducing this)
	    [[nodiscard]] constexpr size_type size(this auto&& self) noexcept {
	        return plg::visit([](auto&& vec) { return vec.size(); }, self.storage_);
	    }

	    [[nodiscard]] constexpr size_type capacity(this auto&& self) noexcept {
	        return plg::visit([](auto&& vec) { return vec.capacity(); }, self.storage_);
	    }

	    [[nodiscard]] constexpr bool empty(this auto&& self) noexcept {
	        return self.size() == 0;
	    }

	    [[nodiscard]] static constexpr size_type max_stack_size() noexcept {
	        return N;
	    }

	    // Element access using C++23 explicit object parameter
	    [[nodiscard]] constexpr reference operator[](this auto&& self, size_type pos) {
	        return plg::visit([pos](auto&& vec) -> reference {
	            return vec[pos];
	        }, self.storage_);
	    }

	    [[nodiscard]] constexpr reference at(this auto&& self, size_type pos) {
	    	return plg::visit([pos](auto&& vec) -> reference {
				return vec.at(pos);
			}, self.storage_);
	    }

	    [[nodiscard]] constexpr reference front(this auto&& self) {
	        return plg::visit([](auto&& vec) -> reference {
	            return vec.front();
	        }, self.storage_);
	    }

	    [[nodiscard]] constexpr reference back(this auto&& self) {
	        return plg::visit([](auto&& vec) -> reference {
	            return vec.back();
	        }, self.storage_);
	    }

	    // Modifiers
	    constexpr void push_back(const T& value) {
	        if (is_small()) {
	            auto& small_vec = plg::get<0>(storage_);
	            if (small_vec.size() < N) {
	                small_vec.push_back(value);
	            } else {
	                promote_to_heap();
	                plg::get<1>(storage_).push_back(value);
	            }
	        } else {
	            plg::get<1>(storage_).push_back(value);
	        }
	    }

	    constexpr void push_back(T&& value) {
	        if (is_small()) {
	            auto& small_vec = plg::get<0>(storage_);
	            if (small_vec.size() < N) {
	                small_vec.push_back(std::move(value));
	            } else {
	                promote_to_heap();
	                plg::get<1>(storage_).push_back(std::move(value));
	            }
	        } else {
	            plg::get<1>(storage_).push_back(std::move(value));
	        }
	    }

	    template<typename... Args>
	    constexpr reference emplace_back(Args&&... args) {
	        if (is_small()) {
	            auto& small_vec = plg::get<0>(storage_);
	            if (small_vec.size() < N) {
	                return small_vec.emplace_back(std::forward<Args>(args)...);
	            } else {
	                promote_to_heap();
	                return plg::get<1>(storage_).emplace_back(std::forward<Args>(args)...);
	            }
	        } else {
	            return plg::get<1>(storage_).emplace_back(std::forward<Args>(args)...);
	        }
	    }

	    constexpr void pop_back() {
	        plg::visit([](auto&& vec) { vec.pop_back(); }, storage_);
	    }

	    constexpr void clear() noexcept {
	        plg::visit([](auto&& vec) { vec.clear(); }, storage_);
	    }

		// Insert single element (copy)
	    constexpr auto insert(auto pos, const T& value) {
	        if (is_small()) {
	            auto& small_vec = plg::get<0>(storage_);
	            if (small_vec.size() < N) {
	                return small_vec.insert(pos, value);
	            } else {
	                promote_to_heap();
	                return plg::get<1>(storage_).insert(pos, value);
	            }
	        } else {
	            return plg::get<1>(storage_).insert(pos, value);
	        }
	    }

	    // Insert single element (move)
	    constexpr auto insert(auto pos, T&& value) {
	        if (is_small()) {
	            auto& small_vec = plg::get<0>(storage_);
	            if (small_vec.size() < N) {
	                return small_vec.insert(pos, std::move(value));
	            } else {
	                promote_to_heap();
	                return plg::get<1>(storage_).insert(pos, std::move(value));
	            }
	        } else {
	            return plg::get<1>(storage_).insert(pos, std::move(value));
	        }
	    }

	    // Insert count copies
	    constexpr auto insert(auto pos, size_type count, const T& value) {
	        if (is_small()) {
	            auto& small_vec = plg::get<0>(storage_);
	            if (small_vec.size() + count <= N) {
	                return small_vec.insert(pos, count, value);
	            } else {
	                promote_to_heap();
	                return plg::get<1>(storage_).insert(pos, count, value);
	            }
	        } else {
	            return plg::get<1>(storage_).insert(pos, count, value);
	        }
	    }

	    // Insert range
	    template<std::input_iterator It>
	    constexpr auto insert(auto pos, It first, It last) {
	        if constexpr (std::random_access_iterator<It>) {
	            auto count = std::distance(first, last);
	            if (is_small()) {
	                auto& small_vec = plg::get<0>(storage_);
	                if (small_vec.size() + count <= N) {
	                    return small_vec.insert(pos, first, last);
	                } else {
	                    promote_to_heap();
	                    return plg::get<1>(storage_).insert(pos, first, last);
	                }
	            } else {
	                return plg::get<1>(storage_).insert(pos, first, last);
	            }
	        } else {
	            // For non-random-access iterators, insert one by one
	            auto result = pos;
	            for (; first != last; ++first) {
	                result = insert(result, *first);
	                ++result;
	            }
	            return result;
	        }
	    }

	    // Insert initializer list
	    constexpr auto insert(auto pos, std::initializer_list<T> ilist) {
	        return insert(pos, ilist.begin(), ilist.end());
	    }

		template<std::ranges::input_range R>
		 constexpr void append_range(R&& range) {
	    	for (auto&& elem : range) {
	    		push_back(std::forward<decltype(elem)>(elem));
	    	}
	    }

		// Append from iterator range
		template<std::input_iterator It>
		constexpr void append(It first, It last) {
	    	for (; first != last; ++first) {
	    		push_back(*first);
	    	}
	    }

		// Append from initializer list
		constexpr void append(std::initializer_list<T> ilist) {
	    	append(ilist.begin(), ilist.end());
	    }

		// Erase single element
		constexpr auto erase(auto pos) {
	    	return plg::visit([pos](auto&& vec) { return vec.erase(pos);  }, storage_);
	    }

		// Swap contents with another hybrid_vector
		constexpr void swap(hybrid_vector& other) noexcept {
	    	storage_.swap(other.storage_);
	    }

		// Non-member swap for ADL
		friend constexpr void swap(hybrid_vector& lhs, hybrid_vector& rhs) noexcept {
	    	lhs.swap(rhs);
	    }

		// Reserve capacity
		constexpr void reserve(size_type new_cap) {
	    	if (new_cap <= N) {
	    		// Stack storage already has capacity N, nothing to do
	    		return;
	    	}

	    	// Need heap storage for capacity > N
	    	if (is_small()) {
	    		promote_to_heap();
	    	}

	    	plg::get<1>(storage_).reserve(new_cap);
	    }

		// Erase range
		constexpr auto erase(auto first, auto last) {
	    	return plg::visit([first, last](auto&& vec) { return vec.erase(first, last); }, storage_);
	    }

	    // Iterators
	    [[nodiscard]] constexpr auto begin(this auto&& self) {
	        return plg::visit([](auto&& vec) { return vec.begin(); }, self.storage_);
	    }

	    [[nodiscard]] constexpr auto end(this auto&& self) {
	        return plg::visit([](auto&& vec) { return vec.end(); }, self.storage_);
	    }

	    [[nodiscard]] constexpr auto cbegin(this auto&& self) {
	        return plg::visit([](auto&& vec) { return vec.cbegin(); }, self.storage_);
	    }

	    [[nodiscard]] constexpr auto cend(this auto&& self) {
	        return plg::visit([](auto&& vec) { return vec.cend(); }, self.storage_);
	    }

	    // Range support (C++23)
	    [[nodiscard]] friend constexpr auto begin(hybrid_vector& v) {
	        return v.begin();
	    }

	    [[nodiscard]] friend constexpr auto end(hybrid_vector& v) {
	        return v.end();
	    }

	    [[nodiscard]] friend constexpr auto cbegin(hybrid_vector& v) {
	        return v.cbegin();
	    }

	    [[nodiscard]] friend constexpr auto cend(hybrid_vector& v) {
	        return v.cend();
	    }
	};
}