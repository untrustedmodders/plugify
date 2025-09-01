#pragma once

#include <span>
#include <string>
#include <vector>
#include <memory>

#include "plugify/global.h"

namespace plugify {
	class EnumValue;

	// Enum Class
	class PLUGIFY_API EnumObject {
	    struct Impl;
	public:
	    EnumObject();
	    ~EnumObject();
	    EnumObject(const EnumObject& other);
	    EnumObject(EnumObject&& other) noexcept;
	    EnumObject& operator=(const EnumObject& other);
	    EnumObject& operator=(EnumObject&& other) noexcept;

	    // Getters
	    [[nodiscard]] const std::string& GetName() const noexcept;
	    [[nodiscard]] const std::vector<EnumValue>& GetValues() const noexcept;

	    // Setters (pass by value and move)
	    void SetName(std::string name);
	    void SetValues(std::vector<EnumValue> values);

		[[nodiscard]] bool operator==(const EnumObject& other) const noexcept;
		[[nodiscard]] auto operator<=>(const EnumObject& other) const noexcept;

	PLUGIFY_ACCESS:
	    PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
