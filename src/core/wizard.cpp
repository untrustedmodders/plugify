#include "plugin_manager.h"
#include <wizard/wizard.h>

namespace wizard {
	class Wizard final : public IWizard {
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

			WIZARD_LOG("Wizard Init!", ErrorLevel::INFO);
			WIZARD_LOG("Git: [" WIZARD_GIT_COMMIT_HASH "]:(" WIZARD_GIT_TAG ") - " WIZARD_GIT_COMMIT_SUBJECT " on " WIZARD_GIT_BRANCH " at '" WIZARD_GIT_COMMIT_DATE "'", ErrorLevel::INFO);
			WIZARD_LOG("Compiled on: " WIZARD_COMPILED_SYSTEM " from: " WIZARD_COMPILED_GENERATOR" with: '" WIZARD_COMPILED_COMPILER "'", ErrorLevel::INFO);

			return true;
		}

		void Terminate() override {
			if (!_inited) {
				return;
			}

			_inited = false;

			WIZARD_LOG("Wizard Terminated!", ErrorLevel::INFO);
		}

		void SetLogger(std::shared_ptr<ILogger> logger) override {
			LogSystem::SetLogger(std::move(logger));
		}

	private:
		bool _inited{ false };
	};

	std::shared_ptr<IWizard> MakeWizard() {
		return std::make_shared<Wizard>();
	}
}
