#pragma once

#include <type_traits>

namespace plugify {
	// TODO: Write comments
	template<typename T>
	class Ref {
	public:
		Ref() noexcept = default;
		Ref(T& impl) noexcept : _impl{std::addressof(impl)} {}

		Ref(Ref const&) = default;
		Ref(Ref&&) = default;

		bool operator==(const Ref& other) const noexcept { return _impl == other._impl; }
		bool operator==(const T* impl) const noexcept { return _impl == impl; }

		Ref& operator=(const Ref&) & = default;
		Ref& operator=(const Ref&) && = delete;
		Ref& operator=(Ref&&) & = default;
		Ref& operator=(Ref&&) && = delete;

	protected:
		T* _impl;
	};

	//template<typename T> static constexpr bool is_pod_v = std::is_standard_layout_v<T> and std::is_trivial_v<T>; // C++20 deprecated std::is_ref_v so we must do this
	template<typename T> static constexpr bool is_ref_v = std::is_standard_layout_v<T> and sizeof(T) == sizeof(void*);
}

