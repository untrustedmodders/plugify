#pragma once

#include <memory>

namespace wizard {
	class ILogger;

	class IWizard {
	public:
		virtual ~IWizard() = default;

		virtual bool Initialize() = 0;
		virtual void Terminate() = 0;

		virtual void SetLogger(std::shared_ptr<ILogger> logger) = 0;
	};

	// Entry Point
	std::shared_ptr<IWizard> MakeWizard();
}
