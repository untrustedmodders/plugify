#include "plugify/core/conflict.hpp"
#include "plugify/core/constraint.hpp"

using namespace plugify;

// Conflict Implementation
struct Conflict::Impl {
	PackageId name;
	std::optional<std::vector<Constraint>> constraints;
	std::optional<std::string> reason;
};

Conflict::Conflict() : _impl(std::make_unique<Impl>()) {}
Conflict::~Conflict() = default;

Conflict::Conflict(const Conflict& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {}

Conflict::Conflict(Conflict&& other) noexcept = default;

Conflict& Conflict::operator=(const Conflict& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

Conflict& Conflict::operator=(Conflict&& other) noexcept = default;

const PackageId& Conflict::GetName() const noexcept { return _impl->name; }
std::optional<std::vector<Constraint>> Conflict::GetConstraints() const noexcept {
	return _impl->constraints;
}
std::optional<std::string> Conflict::GetReason() const noexcept { return _impl->reason; }

void Conflict::SetName(PackageId name) noexcept { _impl->name = std::move(name); }
void Conflict::SetConstraints(std::optional<std::vector<Constraint>> constraints) noexcept {
	_impl->constraints = std::move(constraints);
}
void Conflict::SetReason(std::optional<std::string> reason) noexcept {
	_impl->reason = std::move(reason);
}
