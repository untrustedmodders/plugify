#include "core/dependency_impl.hpp"

using namespace plugify;

// Dependency Implementation
Dependency::Dependency() : _impl(std::make_unique<Impl>()) {}
Dependency::~Dependency() = default;

Dependency::Dependency(const Dependency& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {}

Dependency::Dependency(Dependency&& other) noexcept = default;

Dependency& Dependency::operator=(const Dependency& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

Dependency& Dependency::operator=(Dependency&& other) noexcept = default;

const std::string& Dependency::GetName() const noexcept { return _impl->name; }
Constraint Dependency::GetConstraints() const noexcept {
	return _impl->constraints.value_or(Constraint{});
}
bool Dependency::IsOptional() const noexcept { return _impl->optional.value_or(false); }

void Dependency::SetName(std::string name) noexcept { _impl->name = std::move(name); }
void Dependency::SetConstraints(Constraint constraints) noexcept {
	_impl->constraints = std::move(constraints);
}
void Dependency::SetOptional(bool optional) noexcept {
	_impl->optional = optional;
}

bool Dependency::operator==(const Dependency& other) const noexcept = default;
auto Dependency::operator<=>(const Dependency& other) const noexcept = default;
