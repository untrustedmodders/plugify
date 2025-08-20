#include "plugify/core/method.hpp"
#include "plugify/core/property.hpp"

using namespace plugify;

// Method Implementation
struct Method::Impl {
	std::vector<Property> paramTypes;
	Property retType;
	uint8_t varIndex{kNoVarArgs};
	std::string name;
	std::string funcName;
	std::string callConv;
};

Method::Method() : _impl(std::make_unique<Impl>()) {}
Method::~Method() = default;

Method::Method(const Method& other)
    : _impl(std::make_unique<Impl>(*other._impl)) {}

Method::Method(Method&& other) noexcept = default;

Method& Method::operator=(const Method& other) {
    if (this != &other) {
        _impl = std::make_unique<Impl>(*other._impl);
    }
    return *this;
}

Method& Method::operator=(Method&& other) noexcept = default;

const std::vector<Property>& Method::GetParamTypes() const noexcept { return _impl->paramTypes; }
const Property& Method::GetRetType() const noexcept { return _impl->retType; }
uint8_t Method::GetVarIndex() const noexcept { return _impl->varIndex; }
const std::string& Method::GetName() const noexcept { return _impl->name; }
const std::string& Method::GetFuncName() const noexcept { return _impl->funcName; }
const std::string& Method::GetCallConv() const noexcept { return _impl->callConv; }

void Method::SetParamTypes(std::vector<Property> paramTypes) noexcept {
    _impl->paramTypes = std::move(paramTypes);
}
void Method::SetRetType(Property retType) noexcept { _impl->retType = std::move(retType); }
void Method::SetVarIndex(uint8_t varIndex) noexcept { _impl->varIndex = varIndex; }
void Method::SetName(std::string name) noexcept { _impl->name = std::move(name); }
void Method::SetFuncName(std::string funcName) noexcept { _impl->funcName = std::move(funcName); }
void Method::SetCallConv(std::string callConv) noexcept { _impl->callConv = std::move(callConv); }

std::shared_ptr<Method> Method::FindPrototype(std::string_view name) const noexcept {
	for (const auto& param : GetParamTypes()) {
		if (auto method = param.GetPrototype()) {
			if (method->GetName() == name) {
				return method;
			}
			if (auto prototype = method->FindPrototype(name)) {
				return prototype;
			}
		}
	}

	if (auto method = GetRetType().GetPrototype()) {
		if (method->GetName() == name) {
			return method;
		}
		if (auto prototype = method->FindPrototype(name)) {
			return prototype;
		}
	}

	return {};
}
