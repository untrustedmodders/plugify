#include "plugify_provider.hpp"
#include "plugin_manager.hpp"
#include <plugify/api/plugify.hpp>
#include <plugify/api/config.hpp>
#include <util/json.hpp>

namespace plugify {
	class Plugify final : public IPlugify, public std::enable_shared_from_this<Plugify> {
	public:
		Plugify() = default;
		~Plugify() override {
			Terminate();
		};

		bool Initialize(Config& config) override {
			if (IsInitialized())
				return false;

			_configPath = rootDir / "plugify.pconfig";
			PL_LOG_DEBUG("Config path: '{}'", _configPath.string());
			auto json = FileSystem::ReadText(_configPath);
			auto config = glz::read_jsonc<Config>(json);
			if (!config.has_value()) {
				PL_LOG_ERROR("Config: '{}' has JSON parsing error: {}", _configPath.string(), glz::format_error(config.error(), json));
				return false;
			}

			{
				const auto checkPath = [](const fs::path& p) {
					return !p.empty() && p.lexically_normal() == p;
				};

				if (!checkPath(config->configsDir)) {
					PL_LOG_ERROR("Config configsDir must be relative directory path");
					return false;
				}
				if (!checkPath(config->dataDir)) {
					PL_LOG_ERROR("Config dataDir must be relative directory path");
					return false;
				}
				if (!checkPath(config->logsDir)) {
					PL_LOG_ERROR("Config logsDir must be relative directory path");
					return false;
				}
				std::array<fs::path, 5> dirs = {
					"modules",
					"plugins",
					config->configsDir,
					config->dataDir,
					config->logsDir,
				};

				const auto isPathCollides = [](const fs::path& first, const fs::path& second) {
					auto [itFirst, itSecond] = std::mismatch(first.begin(), first.end(), second.begin(), second.end());
					return itFirst == first.end() || itSecond == second.end();
				};

				for (auto first = dirs.begin(); first != dirs.end(); ++first) {
					for (auto second = first + 1; second != dirs.end(); ++second) {
						if (isPathCollides(*first, *second)) {
							PL_LOG_ERROR("Config configsDir, dataDir, logsDir must not share paths with each other or with 'modules', 'plugins'");
							return false;
						}
					}
				}
			}

			_config = std::move(*config);

			if (!rootDir.empty())
				_config.baseDir = rootDir / _config.baseDir;

			_provider = std::make_shared<PlugifyProvider>(weak_from_this());
			_pluginManager = std::make_shared<PluginManager>(weak_from_this());

			_inited = true;

			PL_LOG_INFO("Plugify Init!");
			PL_LOG_INFO("Version: {}", _version);
			PL_LOG_INFO("Git: [{}]:({}) - {} on {} at '{}'", PLUGIFY_GIT_COMMIT_HASH, PLUGIFY_GIT_TAG, PLUGIFY_GIT_COMMIT_SUBJECT, PLUGIFY_GIT_BRANCH, PLUGIFY_GIT_COMMIT_DATE); //-V001
			PL_LOG_INFO("Compiled on: {} from: {} with: '{}'", PLUGIFY_COMPILED_SYSTEM, PLUGIFY_COMPILED_GENERATOR, PLUGIFY_COMPILED_COMPILER);

			return true;
		}

		void Terminate() override {
			if (!IsInitialized())
				return;

			if (_pluginManager.use_count() != 1) {
				PL_LOG_ERROR("Lack of owning for plugin manager! Will not released on plugify terminate");
			}
			_pluginManager.reset();

			_lastTime = DateTime::Now();

			_inited = false;

			PL_LOG_INFO("Plugify Terminated!");
		}

		bool IsInitialized() const override {
			return _inited;
		}

		void Update() override {
			if (!IsInitialized())
				return;

			auto currentTime = DateTime::Now();
			_deltaTime = (currentTime - _lastTime);
			_lastTime = currentTime;

			_pluginManager->Update(_deltaTime);
		}

		void Log(std::string_view msg, Severity severity) override {
			LogSystem::Log(msg, severity);
		}

		void SetLogger(std::shared_ptr<ILogger> logger) override {
			LogSystem::SetLogger(std::move(logger));
		}

		std::weak_ptr<IPluginManager> GetPluginManager() const override {
			return _pluginManager;
		}

		std::weak_ptr<PlugifyProvider> GetProvider() const override {
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
		std::shared_ptr<PlugifyProvider> _provider;
		Version _version{ PLUGIFY_VERSION };
		Config _config;
		DateTime _deltaTime;
		DateTime _lastTime;
		bool _inited{ false };
	};

	std::shared_ptr<IPlugify> MakePlugify() {
		return std::make_shared<Plugify>();
	}
}
