#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "plugify_export.h"

namespace glz {
	template<typename T>
	struct meta;
}

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
	    void SetName(std::string name) noexcept;
	    void SetValue(int64_t value) noexcept;

		[[nodiscard]] bool operator==(const EnumValue& lhs, const EnumValue& rhs) noexcept;

	private:
	    friend struct glz::meta<EnumValue>;
	    std::unique_ptr<Impl> _impl;
	};
}
