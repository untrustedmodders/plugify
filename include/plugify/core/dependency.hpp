#pragma once

#include <span>
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
	class Manager;
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
		[[nodiscard]] std::string_view GetName() const noexcept;
		[[nodiscard]] std::span<const Constraint> GetConstraints() const noexcept;
		[[nodiscard]] bool IsOptional() const noexcept;

		// Setters (pass by value and move)
		void SetName(std::string_view name) noexcept;
		void SetConstraints(std::span<const Constraint> constraints) noexcept;
		void SetOptional(bool optional) noexcept;

		[[nodiscard]] bool operator==(const Dependency& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Dependency& other) const noexcept;

		std::vector<Constraint> GetFailedConstraints(const Version& version) const;

	private:
		friend class Manager;
		friend struct glz::meta<Dependency>;
		//friend struct std::hash<Dependency>;
		std::unique_ptr<Impl> _impl;
	};
}