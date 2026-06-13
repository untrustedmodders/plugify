#pragma once

#include <array>
#include <stdexcept>

namespace plg {
	// Enum-indexed array wrapper
	template<typename T, typename Enum>
	class enum_array {
	    // Convert enum to index
	    static constexpr size_t to_index(Enum e) {
	        return static_cast<size_t>(e);
	    }

		static constexpr size_t enum_size() {
	    	return static_cast<size_t>(Enum::Count);
	    }

		using storage = std::array<T, enum_size()>;
		storage data_;

	public:
	    using value_type = typename storage::value_type;
	    using size_type = typename storage::size_type;
	    using difference_type = typename storage::difference_type;

		using pointer = typename storage::pointer;
		using const_pointer = typename storage::const_pointer;

		using reference = typename storage::reference;
	    using const_reference = typename storage::const_reference;

	    using iterator = typename storage::iterator;
	    using const_iterator = typename storage::const_iterator;
	    using reverse_iterator = typename storage::reverse_iterator;
	    using const_reverse_iterator = typename storage::const_reverse_iterator;

	    constexpr enum_array() = default;

	    constexpr explicit enum_array(const T& default_value) {
	        data_.fill(default_value);
	    }

	    constexpr enum_array(std::initializer_list<T> init) {
	        if (init.size() != size()) {
	            throw std::invalid_argument("Initializer list size must match enum count");
	        }
	        std::copy(init.begin(), init.end(), data_.begin());
	    }

	    constexpr reference operator[](Enum e) {
	        return data_[to_index(e)];
	    }

	    constexpr const_reference operator[](Enum e) const {
	        return data_[to_index(e)];
	    }

	    constexpr reference at(Enum e) {
	        size_t idx = to_index(e);
	        if (idx >= size()) {
	            throw std::out_of_range("Enum value out of range");
	        }
	        return data_[idx];
	    }

	    constexpr const_reference at(Enum e) const {
	        size_t idx = to_index(e);
	        if (idx >= size()) {
	            throw std::out_of_range("Enum value out of range");
	        }
	        return data_[idx];
	    }

		constexpr reference front() noexcept {
	    	return data_.front();
	    }

		constexpr const_reference front() const noexcept {
	    	return data_.front();
	    }

		constexpr reference back() noexcept {
	    	return data_.back();
	    }

		constexpr const_reference back() const noexcept {
	    	return data_.back();
	    }

	    constexpr size_type size() const noexcept {
	        return data_.size();
	    }

	    constexpr size_type max_size() const noexcept {
	        return data_.max_size();
	    }

	    constexpr bool empty() const noexcept {
	        return data_.empty();
	    }

		constexpr iterator begin() noexcept {
	    	return data_.begin();
	    }

		constexpr const_iterator begin() const noexcept {
	    	return data_.begin();
	    }

		constexpr iterator end() noexcept {
	    	return data_.end();
	    }

		constexpr const_iterator end() const noexcept {
	    	return data_.end();
	    }

		constexpr reverse_iterator rbegin() noexcept {
	    	return data_.rbegin();
	    }

		constexpr const_reverse_iterator rbegin() const noexcept {
	    	return data_.rbegin();
	    }

		constexpr reverse_iterator rend() noexcept {
	    	return data_.rend();
	    }

		constexpr const_reverse_iterator rend() const noexcept {
	    	return data_.rend();
	    }

		constexpr const_iterator cbegin() const noexcept {
	    	return data_.cbegin();
	    }

		constexpr const_iterator cend() const noexcept {
	    	return data_.cend();
	    }

		constexpr const_reverse_iterator crbegin() const noexcept {
	    	return data_.crbegin();
	    }

		constexpr const_reverse_iterator crend() const noexcept {
	    	return data_.crend();
	    }

		constexpr pointer data() noexcept {
	    	return data_.data();
	    }

		constexpr const_pointer data() const noexcept {
	    	return data_.data();
	    }

	    constexpr void fill(const T& value) {
	        data_.fill(value);
	    }

		constexpr void swap(enum_array& other) noexcept {
	    	data_.swap(other.data_);
	    }
	};
} // namespace plg
