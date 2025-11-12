#pragma once

#include <memory>
#include <string>

#include "plugify/global.h"

namespace plugify {
	// Alias Class
	class PLUGIFY_API Alias {
	public:
		Alias();
		~Alias();
		Alias(const Alias& other);
		Alias(Alias&& other) noexcept;
		Alias& operator=(const Alias& other);
		Alias& operator=(Alias&& other) noexcept;

		// Getters
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] bool IsOwner() const noexcept;

		// Setters (pass by value and move)
		void SetName(std::string name);
		void SetOwner(bool owner);

		[[nodiscard]] bool operator==(const Alias& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Alias& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
