#include "plugify/core/property.hpp"

using namespace plugify;

// Property Implementation
struct Property::Impl {
	ValueType type{};
	std::optional<bool> ref;
	std::shared_ptr<Method> prototype;
	std::shared_ptr<Enum> enumerate;
};

Property::Property() : _impl(std::make_unique<Impl>()) {}
Property::~Property() = default;

Property::Property(const Property& other)
    : _impl(std::make_unique<Impl>(*other._impl)) {}

Property::Property(Property&& other) noexcept = default;

Property& Property::operator=(const Property& other) {
    if (this != &other) {
        _impl = std::make_unique<Impl>(*other._impl);
    }
    return *this;
}

Property& Property::operator=(Property&& other) noexcept = default;

const ValueType& Property::GetType() const noexcept { return _impl->type; }
std::optional<bool> Property::GetRef() const noexcept { return _impl->ref; }
std::shared_ptr<Method> Property::GetPrototype() const noexcept { return _impl->prototype; }
std::shared_ptr<Enum> Property::GetEnumerate() const noexcept { return _impl->enumerate; }

void Property::SetType(ValueType type) noexcept { _impl->type = type; }
void Property::SetRef(std::optional<bool> ref) noexcept { _impl->ref = ref; }
void Property::SetPrototype(std::shared_ptr<Method> prototype) noexcept {
    _impl->prototype = std::move(prototype);
}
void Property::SetEnumerate(std::shared_ptr<Enum> enumerate) noexcept {
    _impl->enumerate = std::move(enumerate);
}
