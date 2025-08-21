#include "core/property_impl.hpp"

using namespace plugify;

// Property Implementation
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
bool Property::GetRef() const noexcept { return _impl->ref.value_or(false); }
std::shared_ptr<Method> Property::GetPrototype() const noexcept { return _impl->prototype; }
std::shared_ptr<Enum> Property::GetEnumerate() const noexcept { return _impl->enumerate; }

void Property::SetType(ValueType type) noexcept { _impl->type = type; }
void Property::SetRef(bool ref) noexcept { _impl->ref = ref; }
void Property::SetPrototype(std::shared_ptr<Method> prototype) noexcept {
    _impl->prototype = std::move(prototype);
}
void Property::SetEnumerate(std::shared_ptr<Enum> enumerate) noexcept {
    _impl->enumerate = std::move(enumerate);
}

bool Property::operator==(const Property& other) const noexcept = default;
auto Property::operator<=>(const Property& other) const noexcept = default;