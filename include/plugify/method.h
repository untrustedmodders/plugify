#pragma once

#include <vector>
#include <optional>
#include <string_view>
#include <plugify/reference_wrapper.h>
#include <plugify/value_type.h>
#include <plugify_export.h>

// TODO: Write comments
namespace plugify {
#if PLUGIFY_CORE
	struct Method;
	struct Property;
#endif

	class IMethod;

	class PLUGIFY_API IProperty {
		PLUGUFY_REFERENCE(IProperty, const Property)
	public:
		[[nodiscard]] ValueType GetType() const noexcept;
		[[nodiscard]] bool IsReference() const noexcept;
		[[nodiscard]] std::optional<IMethod> GetPrototype() const noexcept;
	};
	static_assert(is_ref_v<IProperty>);

	class PLUGIFY_API IMethod {
		PLUGUFY_REFERENCE(IMethod, const Method)
	public:
		[[nodiscard]] std::string_view GetName() const noexcept;
		[[nodiscard]] std::string_view GetFunctionName() const noexcept;
		[[nodiscard]] std::string_view GetCallingConvention() const noexcept;
		[[nodiscard]] std::vector<IProperty> GetParamTypes() const;
		[[nodiscard]] IProperty GetReturnType() const noexcept;
		[[nodiscard]] uint8_t GetVarIndex() const noexcept;
	};
	static_assert(is_ref_v<IMethod>);
}