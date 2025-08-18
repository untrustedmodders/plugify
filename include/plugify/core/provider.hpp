#pragma once

#include <plugify/core/context.hpp>
#include <plugify/api/assembly.hpp>
#include <plugify/api/constraint.hpp>
#include <plugify/api/file_system.hpp>
#include <plugify/api/provider_handle.hpp>

namespace plugify {
	class Plugify;
	class Provider : public Context {
	public:
		explicit Provider(Plugify& plugify);
		~Provider();

		void Log(std::string_view msg, Severity severity) const;

		const std::filesystem::path& GetBaseDir() const noexcept;

		const std::filesystem::path& GetConfigsDir() const noexcept;

		const std::filesystem::path& GetDataDir() const noexcept;

		const std::filesystem::path& GetLogsDir() const noexcept;

		bool IsPreferOwnSymbols() const noexcept;
		
		bool IsPluginLoaded(std::string_view name, std::optional<Constraint> constraint = {}) const noexcept;
		
		bool IsModuleLoaded(std::string_view name, std::optional<Constraint> constraint = {}) const noexcept;

		PluginHandle FindPlugin(std::string_view name) const noexcept;

		ModuleHandle FindModule(std::string_view name) const noexcept;

		std::shared_ptr<IAssemblyLoader> GetAssemblyLoader() const noexcept;

		std::shared_ptr<IFileSystem> GetFileSystem() const noexcept;
	};
}
