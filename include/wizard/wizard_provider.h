#pragma once

#include <string>

namespace wizard {
	enum class ErrorLevel : uint8_t;

	class IWizardProvider {
	protected:
		~IWizardProvider() = default;

	public:
		virtual void Log(const std::string& msg, ErrorLevel level) = 0;
	};
}
