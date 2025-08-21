#pragma once

#include <span>
#include <vector>
#include <optional>
#include <string>

#include "plugify/core/global.h"
#include "plugify/core/types.hpp"

#include "plugify_export.h"

namespace plugify {
	class Manager;
	struct Constraint;

	// Conflict Class
	class PLUGIFY_API Conflict {
		struct Impl;
	public:
		Conflict();
		~Conflict();
		Conflict(const Conflict& other);
		Conflict(Conflict&& other) noexcept;
		Conflict& operator=(const Conflict& other);
		Conflict& operator=(Conflict&& other) noexcept;

		// Getters
		[[nodiscard]] std::string_view GetName() const noexcept;
		[[nodiscard]] std::span<const Constraint> GetConstraints() const noexcept;
		[[nodiscard]] std::string_view GetReason() const noexcept;

		// Setters (pass by value and move)
		void SetName(std::string_view name) noexcept;
		void SetConstraints(std::span<const Constraint> constraints) noexcept;
		void SetReason(std::string_view reason) noexcept;

		[[nodiscard]] bool operator==(const Conflict& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Conflict& other) const noexcept;

		std::vector<Constraint> GetSatisfiedConstraints(Version version) const;

	PLUGIFY_ACCESS:
		std::unique_ptr<Impl> _impl;
	};
}
