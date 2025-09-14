#include "core/method_impl.hpp"

using namespace plugify;

// Method Implementation
Method::Method()
    : _impl(std::make_unique<Impl>()) {
}

Method::~Method() = default;

Method::Method(const Method& other)
    : _impl(std::make_unique<Impl>(*other._impl)) {
}

Method::Method(Method&& other) noexcept = default;

Method& Method::operator=(const Method& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}

Method& Method::operator=(Method&& other) noexcept = default;

const std::vector<Property>& Method::GetParamTypes() const noexcept {
	return _impl->paramTypes;
}

const Property& Method::GetRetType() const noexcept {
	return _impl->retType;
}

const std::string& Method::GetName() const noexcept {
	return _impl->name;
}

const std::string& Method::GetFuncName() const noexcept {
	return _impl->funcName;
}

CallConv Method::GetCallConv() const noexcept {
	return _impl->callConv.value_or(CallConv::CDecl);
}

uint8_t Method::GetVarIndex() const noexcept {
	return _impl->varIndex.value_or(Signature::kNoVarArgs);
}

void Method::SetParamTypes(std::vector<Property> paramTypes) {
	_impl->paramTypes = std::move(paramTypes);
}

void Method::SetRetType(Property retType) {
	_impl->retType = std::move(retType);
}

void Method::SetName(std::string name) {
	_impl->name = std::move(name);
}

void Method::SetFuncName(std::string funcName) {
	_impl->funcName = std::move(funcName);
}

void Method::SetCallConv(CallConv callConv) {
	_impl->callConv = callConv;
}

void Method::SetVarIndex(uint8_t varIndex) {
	_impl->varIndex = varIndex;
}

bool Method::operator==(const Method& other) const noexcept = default;
auto Method::operator<=>(const Method& other) const noexcept = default;

const Method* Method::FindPrototype(std::string_view name) const noexcept {
	for (const auto& param : GetParamTypes()) {
		if (const auto* method = param.GetPrototype()) {
			if (method->GetName() == name) {
				return method;
			}
			if (auto prototype = method->FindPrototype(name)) {
				return prototype;
			}
		}
	}

	if (const auto* method = GetRetType().GetPrototype()) {
		if (method->GetName() == name) {
			return method;
		}
		if (auto prototype = method->FindPrototype(name)) {
			return prototype;
		}
	}

	return {};
}
