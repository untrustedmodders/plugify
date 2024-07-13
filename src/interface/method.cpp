#include <plugify/method.h>
#include <core/method.h>

using namespace plugify;

ValueType IProperty::GetType() const noexcept {
	return _impl->type;
}

bool IProperty::IsReference() const noexcept {
	return _impl->ref;
}

std::optional<IMethod> IProperty::GetPrototype() const noexcept {
	if (_impl->prototype) {
		return { *(_impl->prototype) };
	} else {
		return {};
	}
}

std::string_view IMethod::GetName() const noexcept {
	return _impl->name;
}

std::string_view IMethod::GetFunctionName() const noexcept {
	return _impl->funcName;
}

std::string_view IMethod::GetCallingConvention() const noexcept {
	return _impl->callConv;
}

std::vector<plugify::IProperty> IMethod::GetParamTypes() const {
	return { _impl->paramTypes.begin(), _impl->paramTypes.end() };
}

plugify::IProperty IMethod::GetReturnType() const noexcept {
	return { _impl->retType };
}

uint8_t IMethod::GetVarIndex() const noexcept {
	return _impl->varIndex;
}