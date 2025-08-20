#include "plugify/_/conflict_handle.hpp"
#include "plugify/core/conflict.hpp"

using namespace plugify;

const std::string& ConflictHandle::GetName() const noexcept {
	return _impl->name;
}

std::span<const Constraint> ConflictHandle::GetConstrants() const noexcept {
	if (const auto& constraints = _impl->constraints) {
		return *constraints;
	}
	return {};
}

const std::string& ConflictHandle::GetReason() const noexcept {
	return _impl->reason;
}
