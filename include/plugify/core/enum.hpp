#pragma once

#include <string>
#include <vector>
#include <memory>

#include "plugify_export.h"

namespace glz {
	template<typename T>
	struct meta;
}

namespace plugify {
	class EnumValue;

	// Enum Class
	class PLUGIFY_API Enum {
	    struct Impl;
	public:
	    Enum();
	    ~Enum();
	    Enum(const Enum& other);
	    Enum(Enum&& other) noexcept;
	    Enum& operator=(const Enum& other);
	    Enum& operator=(Enum&& other) noexcept;

	    // Getters
	    [[nodiscard]] const std::string& GetName() const noexcept;
	    [[nodiscard]] const std::vector<EnumValue>& GetValues() const noexcept;

	    // Setters (pass by value and move)
	    void SetName(std::string name) noexcept;
	    void SetValues(std::vector<EnumValue> values) noexcept;

		[[nodiscard]] bool operator==(const Enum& lhs, const Enum& rhs) noexcept;

	private:
	    friend struct glz::meta<Enum>;
	    std::unique_ptr<Impl> _impl;
	};
}
