#pragma once

#include <type_traits>

template<typename T>
class Ref {
public:
	Ref() noexcept = default;
	Ref(T& impl) noexcept : _impl{std::addressof(impl)} {}

	bool operator==(const Ref& other) const noexcept { return _impl == other._impl; }
	bool operator==(const T* impl) const noexcept { return _impl == impl; }

private:
	T* _impl;
};

namespace plugify {
	//template<typename T> static constexpr bool is_pod_v = std::is_standard_layout_v<T> and std::is_trivial_v<T>; // C++20 deprecated std::is_ref_v so we must do this
	template<typename T> static constexpr bool is_ref_v = std::is_standard_layout_v<T> and sizeof(T) == sizeof(void*);
}

