#pragma once

#include <functional>
#include <type_traits>

namespace plugify {
	class ScopeGuard {
	public:
		ScopeGuard() : f(nullptr) {}
		~ScopeGuard() { if (f) f(); }
		ScopeGuard(const ScopeGuard&) = delete;
		template<class Callable> requires (!std::is_same_v<Callable, ScopeGuard>)
		explicit ScopeGuard(Callable&& undo) : f(std::forward<Callable>(undo)) {}
		ScopeGuard(ScopeGuard&& other) noexcept : f(std::move(other.f)) { other.f = nullptr; }

		ScopeGuard& operator=(const ScopeGuard&) = delete;
		ScopeGuard& operator=(ScopeGuard&& other) noexcept {
			if (f) f();
			f = std::move(other.f);
			other.f = nullptr;
			return *this;
		}
		template<class Callable> requires (!std::is_same_v<Callable, ScopeGuard>)
		ScopeGuard& operator=(Callable&& undo) {
			if (f) f();
			f = std::forward<Callable>(undo);
			return *this;
		}

	private:
		std::function<void()> f;
	};
};