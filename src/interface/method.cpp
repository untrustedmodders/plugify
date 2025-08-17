#include <plugify/api/method.hpp>
#include <core/method.hpp>

using namespace plugify;

const std::string& EnumValueHandle::GetName() const noexcept {
	return _impl->name;
}

int64_t EnumValueHandle::GetValue() const noexcept {
	return _impl->value;
}

const std::string& EnumHandle::GetName() const noexcept {
	return _impl->name;
}

std::span<const EnumValueHandle> EnumHandle::GetValues() const noexcept {
	static_assert(sizeof(std::unique_ptr<EnumValue>) == sizeof(EnumValueHandle), "Unique ptr and handle must have the same size");
	return { reinterpret_cast<const EnumValueHandle*>(_impl->values.data()), _impl->values.size() };
}

ValueType PropertyHandle::GetType() const noexcept {
	return _impl->type;
}

bool PropertyHandle::IsReference() const noexcept {
	return _impl->ref.value_or(false);
}

MethodHandle PropertyHandle::GetPrototype() const noexcept {
	if (const auto& prototype = _impl->prototype) {
		return *prototype;
	}
	return {};
}

EnumHandle PropertyHandle::GetEnum() const noexcept {
	if (const auto enumerate = _impl->enumerate) {
		return *enumerate;
	}
	return {};
}

const std::string& MethodHandle::GetName() const noexcept {
	return _impl->name;
}

const std::string& MethodHandle::GetFunctionName() const noexcept {
	return _impl->funcName;
}

const std::string& MethodHandle::GetCallingConvention() const noexcept {
	return _impl->callConv;
}

std::span<const PropertyHandle> MethodHandle::GetParamTypes() const noexcept {
	static_assert(sizeof(std::unique_ptr<Property>) == sizeof(PropertyHandle), "Unique ptr and handle must have the same size");
	return { reinterpret_cast<const PropertyHandle*>(_impl->paramTypes.data()), _impl->paramTypes.size() };
}

PropertyHandle MethodHandle::GetReturnType() const noexcept {
	return _impl->retType;
}

uint8_t MethodHandle::GetVarIndex() const noexcept {
	return _impl->varIndex;
}

MethodHandle MethodHandle::FindPrototype(std::string_view name) const noexcept {
	if (auto prototype = _impl->FindPrototype(name)) {
		return *prototype;
	}
	return {};
}
