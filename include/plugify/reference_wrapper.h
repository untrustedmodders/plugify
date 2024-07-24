#pragma once

#include <type_traits>

#if PLUGIFY_CORE
#define PLUGUFY_REFERENCE(C, I) public: C() noexcept = default; C(I& impl) noexcept : _impl{std::addressof(impl)} {} bool operator==(const C& other) const noexcept { return _impl == other._impl; } bool operator==(I* other) const noexcept { return _impl == other; } private: I* _impl;
#else
#define PLUGUFY_REFERENCE(C, I) private: C() noexcept = default; C(void* impl) noexcept : _impl{impl} {} void* _impl;
#endif

namespace plugify {
	//template<typename T> static constexpr bool is_pod_v = std::is_standard_layout_v<T> and std::is_trivial_v<T>; // C++20 deprecated std::is_ref_v so we must do this
	template<typename T> static constexpr bool is_ref_v = std::is_standard_layout_v<T> and sizeof(T) == sizeof(void*);
}

