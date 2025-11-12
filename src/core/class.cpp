#include "core/class_impl.hpp"

using namespace plugify;

// Class Implementation
Class::Class()
	: _impl(std::make_unique<Impl>()) {
}

Class::~Class() = default;

Class::Class(const Class& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {
}

Class::Class(Class&& other) noexcept = default;

Class& Class::operator=(const Class& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

Class& Class::operator=(Class&& other) noexcept = default;

// Static empty defaults for returning const references to empty containers
static const std::string emptyString;
static const std::vector<std::string> emptyStrings;

const std::string& Class::GetName() const noexcept {
	return _impl->name;
}
ValueType Class::GetType() const noexcept {
	return _impl->type.value_or(ValueType::Pointer);
}
const std::string& Class::GetInvalid() const noexcept {
	return _impl->invalid ? *_impl->invalid : emptyString;
}
const std::vector<std::string>& Class::GetConstructors() const noexcept {
	return _impl->constructors ? *_impl->constructors : emptyStrings;
}
const std::string& Class::GetDestructor() const noexcept {
	return _impl->destructor ? *_impl->destructor : emptyString;
}
const std::vector<Binding>& Class::GetBindings() const noexcept {
	return _impl->bindings;
}

void Class::SetName(std::string name) {
	_impl->name = std::move(name);
}

void Class::SetType(ValueType type) {
	_impl->type = type;
}

void Class::SetInvalid(std::string invalid) {
	_impl->invalid = std::move(invalid);
}

void Class::SetConstructors(std::vector<std::string> constructors) {
	_impl->constructors = std::move(constructors);
}

void Class::SetDestructor(std::string destructor) {
	_impl->destructor = std::move(destructor);
}

void Class::SetBindings(std::vector<Binding> bindings) {
	_impl->bindings = std::move(bindings);
}

bool Class::operator==(const Class& other) const noexcept = default;
auto Class::operator<=>(const Class& other) const noexcept = default;
