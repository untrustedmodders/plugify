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

std::string_view Dependency::GetName() const noexcept { return _impl->name; }
std::span<const Constraint> Dependency::GetConstraints() const noexcept {
	return _impl->constraints.value_or({});
}
bool Dependency::IsOptional() const noexcept { return _impl->optional.value_or(false); }

void Dependency::SetName(std::string_view name) noexcept { _impl->name = name; }
void Dependency::SetConstraints(std::span<const Constraint> constraints) noexcept {
	_impl->constraints = { constraints.begin(), constraints.end() };
}
void Dependency::SetOptional(bool optional) noexcept {
	_impl->optional = optional;
}

bool Dependency::operator==(const Dependency& other) const noexcept = default;
auto Dependency::operator<=>(const Dependency& other) const noexcept = default;

std::vector<Constraint> Dependency::GetFailedConstraints(const Version& version) const {
	if (!_impl->constraints) return {};
	std::vector<Constraint> satisfied;
	std::ranges::copy_if(*_impl->constraints, std::back_inserter(satisfied), [&](const Constraint& c) {
		return !c.IsSatisfiedBy(version);
	});
	return satisfied;
}