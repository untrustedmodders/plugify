#pragma once

#include <vector>
#include <string>
#include <optional>

#include "plugify/core/types.hpp"

#include "plugify_export.h"

namespace glz {
	template<typename T>
	struct meta;
}

namespace plugify {
	struct Constraint;
	// Dependency Class
	class PLUGIFY_API Dependency {
		struct Impl;
	public:
		Dependency();
		~Dependency();
		Dependency(const Dependency& other);
		Dependency(Dependency&& other) noexcept;
		Dependency& operator=(const Dependency& other);
		Dependency& operator=(Dependency&& other) noexcept;

		// Getters
		[[nodiscard]] const PackageId& GetName() const noexcept;
		[[nodiscard]] std::optional<std::vector<Constraint>> GetConstraints() const noexcept;
		[[nodiscard]] std::optional<bool> GetOptional() const noexcept;

		// Setters (pass by value and move)
		void SetName(PackageId name) noexcept;
		void SetConstraints(std::optional<std::vector<Constraint>> constraints) noexcept;
		void SetOptional(std::optional<bool> optional) noexcept;

		[[nodiscard]] bool operator==(const Dependency& lhs, const Dependency& rhs) noexcept;

	private:
		friend struct glz::meta<Dependency>;
		std::unique_ptr<Impl> _impl;
	};
}