#include "core/method_impl.hpp"

using namespace plugify;

// Method Implementation
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

std::span<const Property> Method::GetParamTypes() const noexcept { return _impl->paramTypes; }
const Property& Method::GetRetType() const noexcept { return _impl->retType; }
uint8_t Method::GetVarIndex() const noexcept { return _impl->varIndex; }
std::string_view Method::GetName() const noexcept { return _impl->name; }
std::string_view Method::GetFuncName() const noexcept { return _impl->funcName; }
std::string_view Method::GetCallConv() const noexcept { return _impl->callConv; }

void Method::SetParamTypes(std::span<const Property> paramTypes) noexcept {
    _impl->paramTypes = { paramTypes.begin(), paramTypes.end() };
}
void Method::SetRetType(const Property& retType) noexcept { _impl->retType = retType; }
void Method::SetVarIndex(uint8_t varIndex) noexcept { _impl->varIndex = varIndex; }
void Method::SetName(std::string_view name) noexcept { _impl->name = name; }
void Method::SetFuncName(std::string_view funcName) noexcept { _impl->funcName = funcName; }
void Method::SetCallConv(std::string_view callConv) noexcept { _impl->callConv = callConv; }

bool Method::operator==(const Method& other) const noexcept = default;
auto Method::operator<=>(const Method& other) const noexcept = default;

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

	std::unique_ptr<>

	return {};
}
