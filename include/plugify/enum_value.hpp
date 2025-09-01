#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "plugify/global.h"

namespace plugify {
	class Method;

	// EnumValue Class
	class PLUGIFY_API EnumValue {
	    struct Impl;
	public:
	    EnumValue();
	    ~EnumValue();
	    EnumValue(const EnumValue& other);
	    EnumValue(EnumValue&& other) noexcept;
	    EnumValue& operator=(const EnumValue& other);
	    EnumValue& operator=(EnumValue&& other) noexcept;

	    // Getters
	    [[nodiscard]] const std::string& GetName() const noexcept;
	    [[nodiscard]] int64_t GetValue() const noexcept;

	    // Setters (pass by value and move)
	    void SetName(std::string name);
	    void SetValue(int64_t value);

		[[nodiscard]] bool operator==(const EnumValue& other) const noexcept;
		[[nodiscard]] auto operator<=>(const EnumValue& other) const noexcept;

	PLUGIFY_ACCESS:
	    PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
