#include <core/method.hpp>
#include <plugify/method.hpp>
#include <utils/pointer.hpp>

using namespace plugify;

std::string_view EnumValueHandle::GetName() const noexcept {
	return _impl->name;
}

int64_t EnumValueHandle::GetValue() const noexcept {
	return _impl->value;
}

std::string_view EnumHandle::GetName() const noexcept {
	return _impl->name;
}

std::span<const EnumValueHandle> EnumHandle::GetValues() const noexcept {
	if (!_impl->_values) {
		_impl->_values = make_shared_nothrow<std::vector<EnumValueHandle>>(_impl->values.begin(), _impl->values.end());
	}
	if (_impl->_values) {
		return *_impl->_values;
	} else {
		return {};
	}
}

ValueType PropertyHandle::GetType() const noexcept {
	return _impl->type;
}

bool PropertyHandle::IsReference() const noexcept {
	return _impl->ref.value_or(false);
}

MethodHandle PropertyHandle::GetPrototype() const noexcept {
	if (_impl->prototype) {
		return { *(_impl->prototype) };
	} else {
		return {};
	}
}

EnumHandle PropertyHandle::GetEnum() const noexcept {
	if (_impl->enumerate) {
		return { *(_impl->enumerate) };
	} else {
		return {};
	}
}

std::string_view MethodHandle::GetName() const noexcept {
	return _impl->name;
}

std::string_view MethodHandle::GetFunctionName() const noexcept {
	return _impl->funcName;
}

std::string_view MethodHandle::GetCallingConvention() const noexcept {
	return _impl->callConv;
}

std::span<const plugify::PropertyHandle> MethodHandle::GetParamTypes() const noexcept {
	if (!_impl->_paramTypes) {
		_impl->_paramTypes = make_shared_nothrow<std::vector<PropertyHandle>>(_impl->paramTypes.begin(), _impl->paramTypes.end());
	}
	if (_impl->_paramTypes) {
		return *_impl->_paramTypes;
	} else {
		return {};
	}
}

plugify::PropertyHandle MethodHandle::GetReturnType() const noexcept {
	return { _impl->retType };
}

uint8_t MethodHandle::GetVarIndex() const noexcept {
	return _impl->varIndex;
}

MethodHandle MethodHandle::FindPrototype(std::string_view name) const noexcept {
	auto prototype = _impl->FindPrototype(name);
	if (prototype) {
		return *prototype;
	}
	return {};
};
