#pragma once

#include <map>
#include <string>
#include <memory>

#include "plugify/global.h"

namespace plugify {
	class Alias;

	// Binding Class
	class PLUGIFY_API Binding {
	public:
		Binding();
		~Binding();
		Binding(const Binding& other);
		Binding(Binding&& other) noexcept;
		Binding& operator=(const Binding& other);
		Binding& operator=(Binding&& other) noexcept;

		// Getters
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] bool IsBindSelf() const noexcept;
		[[nodiscard]] const std::map<size_t, Alias>& GetParamAliases() const noexcept;
		[[nodiscard]] const Alias& GetRetAlias() const noexcept;

		// Setters (pass by value and move)
		void SetName(std::string name);
		void SetBindSelf(bool bindSelf);
		void SetParamAliases(std::map<size_t, Alias> paramAliases);
		void SetRetAlias(Alias retAlias);

		[[nodiscard]] bool operator==(const Binding& other) const noexcept;
		[[nodiscard]] auto operator<=>(const Binding& other) const noexcept;

		PLUGIFY_ACCESS : struct Impl;
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
