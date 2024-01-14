#pragma once

#include <wizard/version.h>
#include <wizard/config.h>
#include <wizard_export.h>
#include <filesystem>
#include <cstdint>
#include <memory>
#include <span>

namespace wizard {
	class ILogger;
	class IWizardProvider;
	class IPluginManager;
	class IPackageManager;
	enum class Severity : uint8_t;

	class IWizard {
	public:
		virtual ~IWizard() = default;

		virtual bool Initialize(const std::filesystem::path& configPath = "wizard.wconfig") = 0;
		virtual void Terminate() = 0;

		virtual void SetLogger(std::shared_ptr<ILogger> logger) = 0;
		virtual void Log(const std::string& msg, Severity severity) = 0;

		virtual std::weak_ptr<IWizardProvider> GetProvider() const = 0;
		virtual std::weak_ptr<IPluginManager> GetPluginManager() const = 0;
		virtual std::weak_ptr<IPackageManager> GetPackageManager() const = 0;
		virtual const Config& GetConfig() const = 0;
		virtual Version GetVersion() const = 0;
	};

	// Entry Point
	WIZARD_API std::shared_ptr<IWizard> MakeWizard();
}
