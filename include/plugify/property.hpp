#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "plugify/global.h"
#include "plugify/value_type.hpp"

namespace plugify {
	class EnumObject;
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
	    [[nodiscard]] ValueType GetType() const noexcept;
	    [[nodiscard]] bool IsRef() const noexcept;
	    [[nodiscard]] const Method* GetPrototype() const noexcept;
	    [[nodiscard]] const EnumObject* GetEnumerate() const noexcept;

	    // Setters (pass by value and move)
	    void SetType(ValueType type);
	    void SetRef(bool ref);
	    void SetPrototype(Method prototype);
	    void SetEnumerate(EnumObject enumerate);

		[[nodiscard]] bool operator==(const Property& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Property& other) const noexcept;

	PLUGIFY_ACCESS:
	    PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
