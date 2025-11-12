#include "core/alias_impl.hpp"

using namespace plugify;

// Alias Implementation
Alias::Alias()
	: _impl(std::make_unique<Impl>()) {
}

Alias::~Alias() = default;

Alias::Alias(const Alias& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {
}

Alias::Alias(Alias&& other) noexcept = default;

Alias& Alias::operator=(const Alias& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

Alias& Alias::operator=(Alias&& other) noexcept = default;

const std::string& Alias::GetName() const noexcept {
	return _impl->name;
}

bool Alias::IsOwned() const noexcept {
	return _impl->owned.value_or(false);
}

void Alias::SetName(std::string name) {
	_impl->name = std::move(name);
}

void Alias::SetOwned(bool owned) {
	_impl->owned = owned;
}

bool Alias::operator==(const Alias& other) const noexcept = default;
auto Alias::operator<=>(const Alias& other) const noexcept = default;
