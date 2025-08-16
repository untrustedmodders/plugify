#pragma once

#include <memory>

namespace plugify {
	template <class T, class... Args>
	std::unique_ptr<T> make_unique_nothrow(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...))) {
		return std::unique_ptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
	}

	template <class T, class... Args>
	std::shared_ptr<T> make_shared_nothrow(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...))) {
		return std::shared_ptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
	}

	template<typename To, typename From>
	std::unique_ptr<To> static_unique_cast(std::unique_ptr<From>&& p) {
		return std::unique_ptr<To>{static_cast<To*>(p.release())};
	}
}
