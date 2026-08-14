#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "plugify/global.h"

namespace plugify {
	class Method;

	// Value Class
	class PLUGIFY_API Value {
	public:
		Value();
		~Value();
		Value(const Value& other);
		Value(Value&& other) noexcept;
		Value& operator=(const Value& other);
		Value& operator=(Value&& other) noexcept;

		// Getters
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] int64_t GetValue() const noexcept;

		// Setters (pass by value and move)
		void SetName(std::string name);
		void SetValue(int64_t value);

		[[nodiscard]] bool operator==(const Value& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Value& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
