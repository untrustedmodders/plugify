#include "plugify/core/enum.hpp"
#include "plugify/core/enum_value.hpp"

using namespace plugify;

// Enum Implementation
struct Enum::Impl {
	std::string name;
	std::vector<EnumValue> values;
};

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

const std::string& Enum::GetName() const noexcept { return _impl->name; }
const std::vector<EnumValue>& Enum::GetValues() const noexcept { return _impl->values; }
void Enum::SetName(std::string name) noexcept { _impl->name = std::move(name); }
void Enum::SetValues(std::vector<EnumValue> values) noexcept { _impl->values = std::move(values); }
