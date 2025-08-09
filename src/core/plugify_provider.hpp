#pragma once

#include "plugify_context.hpp"
#include <plugify/api/plugify.hpp>
#include <plugify/api/plugify_provider.hpp>

namespace plugify {
	class PlugifyProvider : public IPlugifyProvider, public PlugifyContext {
	public:
		explicit PlugifyProvider(std::weak_ptr<IPlugify> plugify);
		~PlugifyProvider();

		void Log(std::string_view msg, Severity severity);

		const fs::path& GetBaseDir() noexcept;

		const fs::path& GetConfigsDir() noexcept;

		const fs::path& GetDataDir() noexcept;

		const fs::path& GetLogsDir() noexcept;

		bool IsPreferOwnSymbols() noexcept;
		
		bool IsPluginLoaded(std::string_view name, std::optional<Constraint> constraint = {}) noexcept;
		
		bool IsModuleLoaded(std::string_view name, std::optional<Constraint> constraint = {}) noexcept;

		PluginHandle FindPlugin(std::string_view name) noexcept;

		ModuleHandle FindModule(std::string_view name) noexcept;
	};
}
