#pragma once

#include <span>
#include <string>
#include <vector>
#include <memory>

#include "plugify/core/global.h"
#include "plugify/core/value_type.hpp"

#include "plugify_export.h"

namespace plugify {
	class Property;

	// Method Class
	class PLUGIFY_API Method {
	    struct Impl;
	public:
	    Method();
	    ~Method();
	    Method(const Method& other);
	    Method(Method&& other) noexcept;
	    Method& operator=(const Method& other);
	    Method& operator=(Method&& other) noexcept;

	    // Getters
	    [[nodiscard]] std::span<const Property> GetParamTypes() const noexcept;
	    [[nodiscard]] const Property& GetRetType() const noexcept;
	    [[nodiscard]] uint8_t GetVarIndex() const noexcept;
	    [[nodiscard]] std::string_view GetName() const noexcept;
	    [[nodiscard]] std::string_view GetFuncName() const noexcept;
	    [[nodiscard]] std::string_view GetCallConv() const noexcept;

	    // Setters (pass by value and move)
	    void SetParamTypes(std::span<const Property> paramTypes) noexcept;
	    void SetRetType(const Property& retType) noexcept;
	    void SetVarIndex(uint8_t varIndex) noexcept;
	    void SetName(std::string_view name) noexcept;
	    void SetFuncName(std::string_view funcName) noexcept;
	    void SetCallConv(std::string_view callConv) noexcept;

		[[nodiscard]] bool operator==(const Method& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Method& other) const noexcept;

		static inline const uint8_t kNoVarArgs = 0xFFU;

		[[nodiscard]] std::shared_ptr<Method> FindPrototype(std::string_view name) const noexcept;

	PLUGIFY_ACCESS:
	    std::unique_ptr<Impl> _impl;
	};
}
