#pragma once

#include <plugify/version.h>
#include <plugify/config.h>
#include <plugify_export.h>
#include <filesystem>
#include <cstdint>
#include <memory>
#include <span>

namespace plugify {
	class ILogger;
	class IPlugifyProvider;
	class IPluginManager;
	class IPackageManager;
	enum class Severity : uint8_t;

	class IPlugify {
	public:
		virtual ~IPlugify() = default;

		virtual bool Initialize(const std::filesystem::path& rootDir = {}) = 0;
		virtual void Terminate() = 0;
		virtual bool IsInitialized() const = 0;

		virtual void SetLogger(std::shared_ptr<ILogger> logger) = 0;
		virtual void Log(const std::string& msg, Severity severity) = 0;

		virtual std::weak_ptr<IPlugifyProvider> GetProvider() const = 0;
		virtual std::weak_ptr<IPluginManager> GetPluginManager() const = 0;
		virtual std::weak_ptr<IPackageManager> GetPackageManager() const = 0;
		virtual const Config& GetConfig() const = 0;
		virtual Version GetVersion() const = 0;
	};

	// Entry Point
	PLUGIFY_API std::shared_ptr<IPlugify> MakePlugify();
}
