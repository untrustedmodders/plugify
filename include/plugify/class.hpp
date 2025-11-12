#pragma once

#include <memory>
#include <vector>
#include <string>

#include "plugify/global.h"
#include "plugify/value_type.hpp"
#include "plg/inplace_vector.hpp"

namespace plugify {
	class Binding;

	// Class Class
	class PLUGIFY_API Class {
	public:
		Class();
		~Class();
		Class(const Class& other);
		Class(Class&& other) noexcept;
		Class& operator=(const Class& other);
		Class& operator=(Class&& other) noexcept;

		// Getters
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] ValueType GetType() const noexcept;
		[[nodiscard]] const std::string& GetInvalid() const noexcept;
		[[nodiscard]] const std::vector<std::string>& GetConstructors() const noexcept;
		[[nodiscard]] const std::string& GetDestructor() const noexcept;
		[[nodiscard]] const std::vector<Binding>& GetBindings() const noexcept;

		// Setters (pass by value and move)
		[[nodiscard]] void SetName(std::string name);
		[[nodiscard]] void SetType(ValueType type);
		[[nodiscard]] void SetInvalid(std::string invalid);
		[[nodiscard]] void SetConstructors(std::vector<std::string> constructors);
		[[nodiscard]] void SetDestructor(std::string destructor);
		[[nodiscard]] void SetBindings(std::vector<Binding> bindings);

		[[nodiscard]] bool operator==(const Class& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Class& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
