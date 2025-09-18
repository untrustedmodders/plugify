#include "core/enum_object_impl.hpp"

using namespace plugify;

// Enum Implementation
EnumObject::EnumObject()
	: _impl(std::make_unique<Impl>()) {
}

EnumObject::~EnumObject() = default;

EnumObject::EnumObject(const EnumObject& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {
}

EnumObject::EnumObject(EnumObject&& other) noexcept = default;

EnumObject& EnumObject::operator=(const EnumObject& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

EnumObject& EnumObject::operator=(EnumObject&& other) noexcept = default;

const std::string& EnumObject::GetName() const noexcept {
	return _impl->name;
}

const std::vector<EnumValue>& EnumObject::GetValues() const noexcept {
	return _impl->values;
}

void EnumObject::SetName(std::string name) {
	_impl->name = std::move(name);
}

void EnumObject::SetValues(std::vector<EnumValue> values) {
	_impl->values = std::move(values);
}

bool EnumObject::operator==(const EnumObject& other) const noexcept = default;
auto EnumObject::operator<=>(const EnumObject& other) const noexcept = default;