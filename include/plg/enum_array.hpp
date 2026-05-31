#pragma once

#include <array>
#include <stdexcept>

namespace plg {
	// Enum-indexed array wrapper
	template<typename T, typename Enum>
	class enum_array {
	private:
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
	    using value_type = T;
	    using size_type = size_t;
	    using reference = T&;
	    using const_reference = const T&;
	    using iterator = typename storage::iterator;
	    using const_iterator = typename storage::const_iterator;

	    // Default constructor
	    enum_array() = default;

	    // Initialize with default value
	    explicit enum_array(const T& default_value) {
	        data_.fill(default_value);
	    }

	    // Initialize with initializer list
	    enum_array(std::initializer_list<T> init) {
	        if (init.size() != size()) {
	            throw std::invalid_argument("Initializer list size must match enum count");
	        }
	        std::copy(init.begin(), init.end(), data_.begin());
	    }

	    // Access with enum key (non-const)
	    reference operator[](Enum e) {
	        return data_[to_index(e)];
	    }

	    // Access with enum key (const)
	    const_reference operator[](Enum e) const {
	        return data_[to_index(e)];
	    }

	    // Bounds-checked access
	    reference at(Enum e) {
	        size_t idx = to_index(e);
	        if (idx >= size()) {
	            throw std::out_of_range("Enum value out of range");
	        }
	        return data_[idx];
	    }

	    const_reference at(Enum e) const {
	        size_t idx = to_index(e);
	        if (idx >= size()) {
	            throw std::out_of_range("Enum value out of range");
	        }
	        return data_[idx];
	    }

	    // Size
	    constexpr size_type size() const noexcept {
	        return data_.size();
	    }

	    // Iterators
	    iterator begin() noexcept { return data_.begin(); }
	    const_iterator begin() const noexcept { return data_.begin(); }
	    iterator end() noexcept { return data_.end(); }
	    const_iterator end() const noexcept { return data_.end(); }

	    // Direct access to underlying array
	    storage& array() { return data_; }
	    const storage& array() const { return data_; }

	    // Fill with value
	    void fill(const T& value) {
	        data_.fill(value);
	    }
	};
} // namespace plg
