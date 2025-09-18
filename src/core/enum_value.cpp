#include "core/enum_value_impl.hpp"

using namespace plugify;

// EnumValue Implementation
EnumValue::EnumValue()
	: _impl(std::make_unique<Impl>()) {
}

EnumValue::~EnumValue() = default;

EnumValue::EnumValue(const EnumValue& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {
}

EnumValue::EnumValue(EnumValue&& other) noexcept = default;

EnumValue& EnumValue::operator=(const EnumValue& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

EnumValue& EnumValue::operator=(EnumValue&& other) noexcept = default;

const std::string& EnumValue::GetName() const noexcept {
	return _impl->name;
}

int64_t EnumValue::GetValue() const noexcept {
	return _impl->value;
}

void EnumValue::SetName(std::string name) {
	_impl->name = std::move(name);
}

void EnumValue::SetValue(int64_t value) {
	_impl->value = value;
}

bool EnumValue::operator==(const EnumValue& other) const noexcept = default;
auto EnumValue::operator<=>(const EnumValue& other) const noexcept = default;