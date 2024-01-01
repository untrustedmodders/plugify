#include "wizard_provider.h"
#include "plugin_manager.h"
#include <wizard/version.h>
#include <wizard/wizard.h>
#include <utils/file_system.h>
#include <utils/virtual_file_system.h>
#include <utils/json.h>

namespace wizard {
	class Wizard final : public IWizard, public std::enable_shared_from_this<Wizard> {
	public:
		Wizard() = default;
		~Wizard() override {
			if (_inited) {
				Terminate();
			}
		};

		bool Initialize(std::span<const char*> args) override {
			if (_inited) {
				return false;
			}

            WZ_LOG_INFO("Wizard Init!");
            WZ_LOG_INFO("Version: {}", _version.ToString());
            WZ_LOG_INFO("Git: [{}]:({}) - {} on {} at '{}'", WIZARD_GIT_COMMIT_HASH, WIZARD_GIT_TAG, WIZARD_GIT_COMMIT_SUBJECT, WIZARD_GIT_BRANCH, WIZARD_GIT_COMMIT_DATE);
            WZ_LOG_INFO("Compiled on: {} from: {} with: '{}'", WIZARD_COMPILED_SYSTEM, WIZARD_COMPILED_GENERATOR, WIZARD_COMPILED_COMPILER);

            auto json = FileSystem::ReadText("wizard.wconfig");
            auto config = glz::read_json<Config>(json);
            if (!config.has_value()) {
                WZ_LOG_ERROR("Config: wizard.wconfig has JSON parsing error: {}", glz::format_error(config.error(), json));
                return false;
            }

            _config = std::move(*config);

            VirtualFileSystem::Initialize(args[0]);

			_provider = std::make_shared<WizardProvider>(weak_from_this());
			_pluginManager = std::make_shared<PluginManager>(weak_from_this());

            _inited = true;

			return true;
		}

		void Terminate() override {
			if (!_inited) {
				return;
			}

			if (_pluginManager.use_count() != 1) {
                WZ_LOG_ERROR("Lack of owning for plugin manager! Will not released on wizard terminate");
			}
			_pluginManager.reset();

            VirtualFileSystem::Shutdown();

			_inited = false;

            WZ_LOG_INFO("Wizard Terminated!");
		}

		void Log(const std::string& msg, Severity severity) override {
            LogSystem::Log(msg, severity);
		}

		void SetLogger(std::shared_ptr<ILogger> logger) override {
			LogSystem::SetLogger(std::move(logger));
		}

		std::weak_ptr<IPluginManager> GetPluginManager() const override {
			return _pluginManager;
		}

		std::weak_ptr<IWizardProvider> GetProvider() const override {
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
		std::shared_ptr<WizardProvider> _provider;
        Version _version{ WIZARD_VERSION_MAJOR, WIZARD_VERSION_MINOR, WIZARD_VERSION_PATCH, WIZARD_VERSION_TWEAK };
        Config _config;
        bool _inited{ false };
	};

	std::shared_ptr<IWizard> MakeWizard() {
		return std::make_shared<Wizard>();
	}
}
