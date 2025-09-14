#include "core/conflict_impl.hpp"

using namespace plugify;

// Conflict Implementation
Conflict::Conflict()
    : _impl(std::make_unique<Impl>()) {
}

Conflict::~Conflict() = default;

Conflict::Conflict(const Conflict& other)
    : _impl(std::make_unique<Impl>(*other._impl)) {
}

Conflict::Conflict(Conflict&& other) noexcept = default;

Conflict& Conflict::operator=(const Conflict& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

Conflict& Conflict::operator=(Conflict&& other) noexcept = default;

const std::string& Conflict::GetName() const noexcept {
	return _impl->name;
}

static const Constraint emptyConstraint;

const Constraint& Conflict::GetConstraints() const noexcept {
	return _impl->constraints ? *_impl->constraints : emptyConstraint;
}

static const std::string emptyString;

const std::string& Conflict::GetReason() const noexcept {
	return _impl->reason ? *_impl->reason : emptyString;
}

void Conflict::SetName(std::string name) {
	_impl->name = std::move(name);
}

void Conflict::SetConstraints(Constraint constraints) {
	_impl->constraints = std::move(constraints);
}

void Conflict::SetReason(std::string reason) {
	_impl->reason = std::move(reason);
}

bool Conflict::operator==(const Conflict& other) const noexcept = default;
auto Conflict::operator<=>(const Conflict& other) const noexcept = default;
