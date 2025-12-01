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
static const std::inplace_vector<std::optional<Alias>, Signature::kMaxFuncArgs> emptyAliases;

const std::string& Binding::GetName() const noexcept {
	return _impl->name;
}

const std::string& Binding::GetMethod() const noexcept {
	return _impl->method;
}

bool Binding::IsBindSelf() const noexcept {
	return _impl->bindSelf.value_or(false);
}

const std::inplace_vector<std::optional<Alias>, Signature::kMaxFuncArgs>& Binding::GetParamAliases() const noexcept {
	return _impl->paramAliases ? *_impl->paramAliases : emptyAliases;
}

const std::optional<Alias>& Binding::GetRetAlias() const noexcept {
	return _impl->retAlias;
}

void Binding::SetName(std::string name) {
	_impl->name = std::move(name);
}

void Binding::SetMethod(std::string method) {
	_impl->method = std::move(method);
}

void Binding::SetBindSelf(bool bindSelf) {
	_impl->bindSelf = bindSelf;
}

void Binding::SetParamAliases(std::inplace_vector<std::optional<Alias>, Signature::kMaxFuncArgs> paramAliases) {
	_impl->paramAliases = std::move(paramAliases);
}

void Binding::SetRetAlias(std::optional<Alias> retAlias) {
	_impl->retAlias = std::move(retAlias);
}

bool Binding::operator==(const Binding& other) const noexcept = default;
auto Binding::operator<=>(const Binding& other) const noexcept = default;
