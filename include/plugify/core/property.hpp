#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "plugify/core/global.h"
#include "plugify/core/value_type.hpp"

#include "plugify_export.h"

namespace plugify {
	class Enum;
	class Method;

	// Property Class
	class PLUGIFY_API Property {
	    struct Impl;
	public:
	    Property();
	    ~Property();
	    Property(const Property& other);
	    Property(Property&& other) noexcept;
	    Property& operator=(const Property& other);
	    Property& operator=(Property&& other) noexcept;

	    // Getters
	    [[nodiscard]] const ValueType& GetType() const noexcept;
	    [[nodiscard]] bool GetRef() const noexcept;
	    [[nodiscard]] std::shared_ptr<Method> GetPrototype() const noexcept;
	    [[nodiscard]] std::shared_ptr<Enum> GetEnumerate() const noexcept;

	    // Setters (pass by value and move)
	    void SetType(ValueType type) noexcept;
	    void SetRef(bool ref) noexcept;
	    void SetPrototype(std::shared_ptr<Method> prototype) noexcept;
	    void SetEnumerate(std::shared_ptr<Enum> enumerate) noexcept;

		[[nodiscard]] bool operator==(const Property& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Property& other) const noexcept;

	PLUGIFY_ACCESS:
	    std::unique_ptr<Impl> _impl;
	};
}
