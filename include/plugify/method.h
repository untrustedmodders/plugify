#pragma once

#include <span>
#include <optional>
#include <string_view>
#include <plugify/reference_wrapper.h>
#include <plugify/value_type.h>
#include <plugify_export.h>

// TODO: Write comments
namespace plugify {
	struct Method;
	struct Property;
	class MethodRef;

	class PLUGIFY_API PropertyRef : public Ref<const Property> {
		using Ref::Ref;
	public:
		[[nodiscard]] ValueType GetType() const noexcept;
		[[nodiscard]] bool IsReference() const noexcept;
		[[nodiscard]] std::optional<MethodRef> GetPrototype() const noexcept;
	};
	static_assert(is_ref_v<PropertyRef>);

	class PLUGIFY_API MethodRef : public Ref<const Method> {
		using Ref::Ref;
	public:
		[[nodiscard]] std::string_view GetName() const noexcept;
		[[nodiscard]] std::string_view GetFunctionName() const noexcept;
		[[nodiscard]] std::string_view GetCallingConvention() const noexcept;
		[[nodiscard]] std::span<const PropertyRef> GetParamTypes() const noexcept;
		[[nodiscard]] PropertyRef GetReturnType() const noexcept;
		[[nodiscard]] uint8_t GetVarIndex() const noexcept;
	};
	static_assert(is_ref_v<MethodRef>);
}