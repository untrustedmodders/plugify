#include <plugify/method.h>
#include <core/method.h>
#include <utils/pointer.h>

using namespace plugify;

ValueType PropertyRef::GetType() const noexcept {
	return _impl->type;
}

bool PropertyRef::IsReference() const noexcept {
	return _impl->ref;
}

std::optional<MethodRef> PropertyRef::GetPrototype() const noexcept {
	if (_impl->prototype) {
		return { *(_impl->prototype) };
	} else {
		return {};
	}
}

const std::string& MethodRef::GetName() const noexcept {
	return _impl->name;
}

const std::string& MethodRef::GetFunctionName() const noexcept {
	return _impl->funcName;
}

const std::string& MethodRef::GetCallingConvention() const noexcept {
	return _impl->callConv;
}

std::span<const plugify::PropertyRef> MethodRef::GetParamTypes() const noexcept {
	if (!_impl->_paramTypes) {
		_impl->_paramTypes = make_shared_nothrow<std::vector<PropertyRef>>(_impl->paramTypes.begin(), _impl->paramTypes.end());
	}
	if (_impl->_paramTypes) {
		return *_impl->_paramTypes;
	} else {
		return {};
	}
}

plugify::PropertyRef MethodRef::GetReturnType() const noexcept {
	return { _impl->retType };
}

uint8_t MethodRef::GetVarIndex() const noexcept {
	return _impl->varIndex;
}