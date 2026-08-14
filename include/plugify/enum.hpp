#pragma once

#include <memory>
#include <string>
#include <vector>

#include "plugify/global.h"

namespace plugify {
	class Value;

	// Enum Class
	class PLUGIFY_API Enum {
	public:
		Enum();
		~Enum();
		Enum(const Enum& other);
		Enum(Enum&& other) noexcept;
		Enum& operator=(const Enum& other);
		Enum& operator=(Enum&& other) noexcept;

		// Getters
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] const std::vector<Value>& GetValues() const noexcept;

		// Setters (pass by value and move)
		void SetName(std::string name);
		void SetValues(std::vector<Value> values);

		[[nodiscard]] bool operator==(const Enum& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Enum& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
