#include "wizard_provider.h"
#include "plugin_manager.h"
#include "version.h"
#include <wizard/wizard.h>

namespace wizard {
	class Wizard final : public IWizard, public std::enable_shared_from_this<Wizard> {
	public:
		Wizard() = default;
		~Wizard() override {
			if (_inited) {
				Terminate();
			}
		};

		bool Initialize() override {
			if (_inited) {
				return false;
			}

			_inited = true;

            WZ_LOG_INFO("Wizard Init!");
            WZ_LOG_INFO("Version: {}", version.ToString());
            WZ_LOG_INFO("Git: [{}]:({}) - {} on {} at '{}'", WIZARD_GIT_COMMIT_HASH, WIZARD_GIT_TAG, WIZARD_GIT_COMMIT_SUBJECT, WIZARD_GIT_BRANCH, WIZARD_GIT_COMMIT_DATE);
            WZ_LOG_INFO("Compiled on: {} from: {} with: '{}'", WIZARD_COMPILED_SYSTEM, WIZARD_COMPILED_GENERATOR, WIZARD_COMPILED_COMPILER);

			_provider = std::make_shared<WizardProvider>(weak_from_this());
			_pluginManager = std::make_shared<PluginManager>(weak_from_this());

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

			_inited = false;

            WZ_LOG_INFO("Wizard Terminated!");
		}

		void Log(const std::string& msg, Severity severity) override {
            LogSystem::Log(msg, severity);
		}

		void SetLogger(std::shared_ptr<ILogger> logger) override {
			LogSystem::SetLogger(std::move(logger));
		}

		std::weak_ptr<IPluginManager> GetPluginManager() override {
			return _pluginManager;
		}

		std::weak_ptr<IWizardProvider> GetProvider() override {
			return _provider;
		}

	private:
		bool _inited{ false };
		std::shared_ptr<PluginManager> _pluginManager;
		std::shared_ptr<WizardProvider> _provider;
        Version version{ WIZARD_VERSION_MAJOR, WIZARD_VERSION_MINOR, WIZARD_VERSION_PATCH, WIZARD_VERSION_TWEAK };
	};

	std::shared_ptr<IWizard> MakeWizard() {
		return std::make_shared<Wizard>();
	}
}
