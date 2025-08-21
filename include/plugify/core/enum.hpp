#pragma once

#include <span>
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
	    [[nodiscard]] std::string_view GetName() const noexcept;
	    [[nodiscard]] std::span<const EnumValue> GetValues() const noexcept;

	    // Setters (pass by value and move)
	    void SetName(std::string_view name) noexcept;
	    void SetValues(std::span<const EnumValue> values) noexcept;

		[[nodiscard]] bool operator==(const Enum& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Enum& other) const noexcept;

	private:
	    friend struct glz::meta<Enum>;
		//friend struct std::hash<Enum>;
	    std::unique_ptr<Impl> _impl;
	};
}
