#pragma once

#include <string>
#include <vector>
#include <memory>

#include "value_type.hpp"

#include "plugify_export.h"

namespace glz {
	template<typename T>
	struct meta;
}

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
	    [[nodiscard]] const std::vector<Property>& GetParamTypes() const noexcept;
	    [[nodiscard]] const Property& GetRetType() const noexcept;
	    [[nodiscard]] uint8_t GetVarIndex() const noexcept;
	    [[nodiscard]] const std::string& GetName() const noexcept;
	    [[nodiscard]] const std::string& GetFuncName() const noexcept;
	    [[nodiscard]] const std::string& GetCallConv() const noexcept;

	    // Setters (pass by value and move)
	    void SetParamTypes(std::vector<Property> paramTypes) noexcept;
	    void SetRetType(Property retType) noexcept;
	    void SetVarIndex(uint8_t varIndex) noexcept;
	    void SetName(std::string name) noexcept;
	    void SetFuncName(std::string funcName) noexcept;
	    void SetCallConv(std::string callConv) noexcept;

		[[nodiscard]] bool operator==(const Method& lhs, const Method& rhs) noexcept;

		static inline const uint8_t kNoVarArgs = 0xFFU;

		[[nodiscard]] std::shared_ptr<Method> FindPrototype(std::string_view name) const noexcept;

	private:
	    friend struct glz::meta<Method>;
	    std::unique_ptr<Impl> _impl;
	};
}
