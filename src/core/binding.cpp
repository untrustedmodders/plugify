#include "core/binding_impl.hpp"

using namespace plugify;

// Binding Implementation
Binding::Binding()
	: _impl(std::make_unique<Impl>()) {
}

Binding::~Binding() = default;

Binding::Binding(const Binding& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {
}

Binding::Binding(Binding&& other) noexcept = default;

Binding& Binding::operator=(const Binding& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

Binding& Binding::operator=(Binding&& other) noexcept = default;

// Static empty defaults for returning const references to empty containers
static const std::map<size_t, Alias> emptyAliases;
static const Alias emptyAlias;

const std::string& Binding::GetName() const noexcept {
	return _impl->name;
}

bool Binding::IsBindSelf() const noexcept {
	return _impl->bindSelf.value_or(false);
}

const std::map<size_t, Alias>& Binding::GetParamAliases() const noexcept {
	return _impl->paramAliases ? *_impl->paramAliases : emptyAliases;
}

const Alias& Binding::GetRetAlias() const noexcept {
	return _impl->retAlias ? *_impl->retAlias : emptyAlias;
}

void Binding::SetName(std::string name) {
	_impl->name = std::move(name);
}

void Binding::SetBindSelf(bool bindSelf) {
	_impl->bindSelf = bindSelf;
}

void Binding::SetParamAliases(std::map<size_t, Alias> paramAliases) {
	_impl->paramAliases = std::move(paramAliases);
}

void Binding::SetRetAlias(Alias retAlias) {
	_impl->retAlias = std::move(retAlias);
}

bool Binding::operator==(const Binding& other) const noexcept = default;
auto Binding::operator<=>(const Binding& other) const noexcept = default;
