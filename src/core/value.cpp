#include "core/value_impl.hpp"

using namespace plugify;

// Value Implementation
Value::Value()
	: _impl(std::make_unique<Impl>()) {
}

Value::~Value() = default;

Value::Value(const Value& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {
}

Value::Value(Value&& other) noexcept = default;

Value& Value::operator=(const Value& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

Value& Value::operator=(Value&& other) noexcept = default;

const std::string& Value::GetName() const noexcept {
	return _impl->name;
}

int64_t Value::GetValue() const noexcept {
	return _impl->value;
}

void Value::SetName(std::string name) {
	_impl->name = std::move(name);
}

void Value::SetValue(int64_t value) {
	_impl->value = value;
}

bool Value::operator==(const Value& other) const noexcept = default;
auto Value::operator<=>(const Value& other) const noexcept = default;
