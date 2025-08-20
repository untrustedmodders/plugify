#include "plugify/core/dependency.hpp"
#include "plugify/core/constraint.hpp"

using namespace plugify;

// Dependency Implementation
struct Dependency::Impl {
	PackageId name;
	std::optional<std::vector<Constraint>> constraints;
	std::optional<bool> optional;
};

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

const PackageId& Dependency::GetName() const noexcept { return _impl->name; }
std::optional<std::vector<Constraint>> Dependency::GetConstraints() const noexcept {
	return _impl->constraints;
}
std::optional<bool> Dependency::GetOptional() const noexcept { return _impl->optional; }

void Dependency::SetName(PackageId name) noexcept { _impl->name = std::move(name); }
void Dependency::SetConstraints(std::optional<std::vector<Constraint>> constraints) noexcept {
	_impl->constraints = std::move(constraints);
}
void Dependency::SetOptional(std::optional<bool> optional) noexcept {
	_impl->optional = optional;
}
