#include "core/property_impl.hpp"

using namespace plugify;

// Property Implementation
Property::Property()
	: _impl(std::make_unique<Impl>()) {
}

Property::~Property() = default;

Property::Property(const Property& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {
}

Property::Property(Property&& other) noexcept = default;

Property& Property::operator=(const Property& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

Property& Property::operator=(Property&& other) noexcept = default;

ValueType Property::GetType() const noexcept {
	return _impl->type;
}

bool Property::IsRef() const noexcept {
	return _impl->ref.value_or(false);
}

const Method* Property::GetPrototype() const noexcept {
	return _impl->prototype ? &*_impl->prototype : nullptr;
}

const EnumObject* Property::GetEnumerate() const noexcept {
	return _impl->enumerate ? &*_impl->enumerate : nullptr;
}

void Property::SetType(ValueType type) {
	_impl->type = type;
}

void Property::SetRef(bool ref) {
	_impl->ref = ref;
}

void Property::SetPrototype(Method prototype) {
	_impl->prototype = std::move(prototype);
}

void Property::SetEnumerate(EnumObject enumerate) {
	_impl->enumerate = std::move(enumerate);
}

bool Property::operator==(const Property& other) const noexcept = default;
auto Property::operator<=>(const Property& other) const noexcept = default;