#include "plugify_provider.h"
#include "plugin_manager.h"
#include "package_manager.h"
#include <plugify/version.h>
#include <plugify/plugify.h>
#include <utils/file_system.h>
#include <utils/http_downloader.h>
#include <utils/json.h>

namespace plugify {
	class Plugify final : public IPlugify, public std::enable_shared_from_this<Plugify> {
	public:
		Plugify() = default;
		~Plugify() override {
			Terminate();
		};

		bool Initialize(const fs::path& rootDir) override {
			if (_inited)
				return false;

			_configPath = rootDir / "plugify.pconfig";
			auto json = FileSystem::ReadText(_configPath);
			auto config = glz::read_json<Config>(json);
			if (!config.has_value()) {
				PL_LOG_ERROR("Config: '{}' has JSON parsing error: {}", _configPath.string(), glz::format_error(config.error(), json));
				return false;
			}

			_config = std::move(*config);

			if (!rootDir.empty())
				_config.baseDir = rootDir / _config.baseDir;

			_provider = std::make_shared<PlugifyProvider>(weak_from_this());
			_packageManager = std::make_shared<PackageManager>(weak_from_this());
			_pluginManager = std::make_shared<PluginManager>(weak_from_this());

			_inited = true;

			PL_LOG_INFO("Plugify Init!");
			PL_LOG_INFO("Version: {}", _version.ToString());
			PL_LOG_INFO("Git: [{}]:({}) - {} on {} at '{}'", PLUGIFY_GIT_COMMIT_HASH, PLUGIFY_GIT_TAG, PLUGIFY_GIT_COMMIT_SUBJECT, PLUGIFY_GIT_BRANCH, PLUGIFY_GIT_COMMIT_DATE);
			PL_LOG_INFO("Compiled on: {} from: {} with: '{}'", PLUGIFY_COMPILED_SYSTEM, PLUGIFY_COMPILED_GENERATOR, PLUGIFY_COMPILED_COMPILER);

			return true;
		}

		void Terminate() override {
			if (!_inited)
				return;

			if (_packageManager.use_count() != 1) {
				PL_LOG_ERROR("Lack of owning for package manager! Will not released on plugify terminate");
			}
			_packageManager.reset();

			if (_pluginManager.use_count() != 1) {
				PL_LOG_ERROR("Lack of owning for plugin manager! Will not released on plugify terminate");
			}
			_pluginManager.reset();

			_inited = false;

			PL_LOG_INFO("Plugify Terminated!");
		}

		bool IsInitialized() const override {
			return _inited;
		}

		void Log(const std::string& msg, Severity severity) override {
			LogSystem::Log(msg, severity);
		}

		void SetLogger(std::shared_ptr<ILogger> logger) override {
			LogSystem::SetLogger(std::move(logger));
		}
		
		bool AddRepository(const std::string& repository) override {
			if (!HTTPDownloader::IsValidURL(repository))
				return false;
			
			auto [it, result] = _config.repositories.emplace(repository);
			if (result) {
				return FileSystem::WriteText(_configPath, glz::write_json(_config));
			}

			return false;
		}

		std::weak_ptr<IPluginManager> GetPluginManager() const override {
			return _pluginManager;
		}

		std::weak_ptr<IPackageManager> GetPackageManager() const override {
			return _packageManager;
		}

		std::weak_ptr<IPlugifyProvider> GetProvider() const override {
			return _provider;
		}

		const Config& GetConfig() const override {
			return _config;
		}

		Version GetVersion() const override {
			return _version;
		}

	private:
		std::shared_ptr<PluginManager> _pluginManager;
		std::shared_ptr<PackageManager> _packageManager;
		std::shared_ptr<PlugifyProvider> _provider;
		Version _version{ PLUGIFY_VERSION_MAJOR, PLUGIFY_VERSION_MINOR, PLUGIFY_VERSION_PATCH, PLUGIFY_VERSION_TWEAK };
		Config _config;
		fs::path _configPath;
		bool _inited{ false };
	};

	std::shared_ptr<IPlugify> MakePlugify() {
		return std::make_shared<Plugify>();
	}
}
