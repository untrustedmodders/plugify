#pragma once

#include <span>
#include <vector>
#include <optional>
#include <string>

#include "plugify/global.h"
#include "plugify/types.hpp"

namespace plugify {
	class Manager;

	// Conflict Class
	class PLUGIFY_API Conflict {
	public:
		Conflict();
		~Conflict();
		Conflict(const Conflict& other);
		Conflict(Conflict&& other) noexcept;
		Conflict& operator=(const Conflict& other);
		Conflict& operator=(Conflict&& other) noexcept;

		// Getters
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] const Constraint& GetConstraints() const noexcept;
		[[nodiscard]] const std::string& GetReason() const noexcept;

		// Setters (pass by value and move)
		void SetName(std::string name);
		void SetConstraints(Constraint constraints);
		void SetReason(std::string reason);

		[[nodiscard]] bool operator==(const Conflict& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Conflict& other) const noexcept;

	PLUGIFY_ACCESS:
		struct Impl;
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};

    using Obsolete = Conflict;
}
