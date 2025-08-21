#include "core/conflict_impl.hpp"

using namespace plugify;

// Conflict Implementation
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

std::string_view Conflict::GetName() const noexcept { return _impl->name; }
std::span<const Constraint> Conflict::GetConstraints() const noexcept {
	return _impl->constraints.value_or(std::vector<Constraint>{});
}
std::string_view Conflict::GetReason() const noexcept { return _impl->reason.value_or(""); }

void Conflict::SetName(std::string_view name) noexcept { _impl->name = name; }
void Conflict::SetConstraints(std::span<const Constraint> constraints) noexcept {
	_impl->constraints = { constraints.begin(), constraints.end() };
}
void Conflict::SetReason(std::string_view reason) noexcept {
	_impl->reason = reason;
}

bool Conflict::operator==(const Conflict& other) const noexcept = default;
auto Conflict::operator<=>(const Conflict& other) const noexcept = default;

std::vector<Constraint> Conflict::GetSatisfiedConstraints(Version version) const {
	if (!_impl->constraints) return {};
	std::vector<Constraint> satisfied;
	std::ranges::copy_if(*_impl->constraints, std::back_inserter(satisfied), [&](const Constraint& c) {
		return c.IsSatisfiedBy(version);
	});
	return satisfied;
}