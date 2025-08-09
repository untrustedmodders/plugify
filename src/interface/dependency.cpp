#include <core/dependency.hpp>
#include <plugify/api/dependency.hpp>
#include <util/pointer.hpp>

using namespace plugify;

std::string_view DependencyHandle::GetName() const noexcept {
	return _impl->name;
}

std::span<const Constraint> DependencyHandle::GetConstrants() const noexcept {
	if (_impl->constraints) {
		return *_impl->constraints;
	}
	return {};
}

bool DependencyHandle::IsOptional() const noexcept {
	return _impl->optional.value_or(false);
}
