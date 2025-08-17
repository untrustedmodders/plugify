#include <core/dependency.hpp>
#include <plugify/api/dependency.hpp>

using namespace plugify;

const std::string& DependencyHandle::GetName() const noexcept {
	return _impl->name;
}

std::span<const Constraint> DependencyHandle::GetConstrants() const noexcept {
	if (const auto& constraints = _impl->constraints) {
		return *constraints;
	}
	return {};
}

bool DependencyHandle::IsOptional() const noexcept {
	return _impl->optional.value_or(false);
}
