#pragma once

#include "plugify/core/types.hpp"

#include "plugify_export.h"

namespace glz {
	template<typename T>
	struct meta;
}

namespace plugify {
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
		[[nodiscard]] const PackageId& GetName() const noexcept;
		[[nodiscard]] std::optional<std::vector<Constraint>> GetConstraints() const noexcept;
		[[nodiscard]] std::optional<std::string> GetReason() const noexcept;

		// Setters (pass by value and move)
		void SetName(PackageId name) noexcept;
		void SetConstraints(std::optional<std::vector<Constraint>> constraints) noexcept;
		void SetReason(std::optional<std::string> reason) noexcept;

		[[nodiscard]] bool operator==(const Conflict& lhs, const Conflict& rhs) noexcept;

	private:
		friend struct glz::meta<Conflict>;
		std::unique_ptr<Impl> _impl;
	};
}
