#include <core/method.hpp>
#include <plugify/method.hpp>
#include <utils/pointer.hpp>

using namespace plugify;

std::string_view EnumValueRef::GetName() const noexcept {
	return _impl->name;
}

int64_t EnumValueRef::GetValue() const noexcept {
	return _impl->value;
}

std::string_view EnumRef::GetName() const noexcept {
	return _impl->name;
}

std::span<const EnumValueRef> EnumRef::GetValues() const noexcept {
	if (!_impl->_values) {
		_impl->_values = make_shared_nothrow<std::vector<EnumValueRef>>(_impl->values.begin(), _impl->values.end());
	}
	if (_impl->_values) {
		return *_impl->_values;
	} else {
		return {};
	}
}

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

std::optional<EnumRef> PropertyRef::GetEnum() const noexcept {
	if (_impl->enumerate) {
		return { *(_impl->enumerate) };
	} else {
		return {};
	}
}

std::string_view MethodRef::GetName() const noexcept {
	return _impl->name;
}

std::string_view MethodRef::GetFunctionName() const noexcept {
	return _impl->funcName;
}

std::string_view MethodRef::GetCallingConvention() const noexcept {
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

std::optional<MethodRef> MethodRef::FindPrototype(std::string_view name) const noexcept {
	auto prototype = _impl->FindPrototype(name);
	if (prototype) {
		return *prototype;
	}
	return {};
};
