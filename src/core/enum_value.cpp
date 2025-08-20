#include "plugify/core/enum_value.hpp"

using namespace plugify;

// EnumValue Implementation
struct EnumValue::Impl {
	std::string name;
	int64_t value{0};
};

EnumValue::EnumValue() : _impl(std::make_unique<Impl>()) {}
EnumValue::~EnumValue() = default;

EnumValue::EnumValue(const EnumValue& other)
    : _impl(std::make_unique<Impl>(*other._impl)) {}

EnumValue::EnumValue(EnumValue&& other) noexcept = default;

EnumValue& EnumValue::operator=(const EnumValue& other) {
    if (this != &other) {
        _impl = std::make_unique<Impl>(*other._impl);
    }
    return *this;
}

EnumValue& EnumValue::operator=(EnumValue&& other) noexcept = default;

const std::string& EnumValue::GetName() const noexcept { return _impl->name; }
int64_t EnumValue::GetValue() const noexcept { return _impl->value; }
void EnumValue::SetName(std::string name) noexcept { _impl->name = std::move(name); }
void EnumValue::SetValue(int64_t value) noexcept { _impl->value = value; }
