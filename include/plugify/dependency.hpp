#pragma once

#include <optional>
#include <span>
#include <string>
#include <vector>

#include "plugify/global.h"
#include "plugify/types.hpp"

namespace plugify {
	class Manager;

	// Dependency Class
	class PLUGIFY_API Dependency {
	public:
		Dependency();
		~Dependency();
		Dependency(const Dependency& other);
		Dependency(Dependency&& other) noexcept;
		Dependency& operator=(const Dependency& other);
		Dependency& operator=(Dependency&& other) noexcept;

		// Getters
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] const Constraint& GetConstraints() const noexcept;
		[[nodiscard]] bool IsOptional() const noexcept;

		// Setters (pass by value and move)
		void SetName(std::string name);
		void SetConstraints(Constraint constraints);
		void SetOptional(bool optional);

		[[nodiscard]] bool operator==(const Dependency& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Dependency& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}