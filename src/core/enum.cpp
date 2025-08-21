#include "core/enum_impl.hpp"

using namespace plugify;

// Enum Implementation
Enum::Enum() : _impl(std::make_unique<Impl>()) {}
Enum::~Enum() = default;

Enum::Enum(const Enum& other)
    : _impl(std::make_unique<Impl>(*other._impl)) {}

Enum::Enum(Enum&& other) noexcept = default;

Enum& Enum::operator=(const Enum& other) {
    if (this != &other) {
        _impl = std::make_unique<Impl>(*other._impl);
    }
    return *this;
}

Enum& Enum::operator=(Enum&& other) noexcept = default;

std::string_view Enum::GetName() const noexcept { return _impl->name; }
std::span<const EnumValue> Enum::GetValues() const noexcept { return _impl->values; }
void Enum::SetName(std::string_view name) noexcept { _impl->name = name; }
void Enum::SetValues(std::span<const EnumValue> values) noexcept { _impl->values = { values.begin(), values.end() }; }

bool Enum::operator==(const Enum& other) const noexcept = default;
auto Enum::operator<=>(const Enum& other) const noexcept = default;