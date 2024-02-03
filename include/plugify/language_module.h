#pragma once

#include <variant>
#include <string>
#include <vector>
#include <memory>

namespace plugify {
	class IPlugin;
	class IModule;
	class IPlugifyProvider;

	using MethodData = std::pair<std::string, void*>;

	struct ErrorData {
		std::string error;
	};

	struct InitResultData {};

	struct LoadResultData {
		std::vector<MethodData> methods;
	};

	using InitResult = std::variant<InitResultData, ErrorData>;
	using LoadResult = std::variant<LoadResultData, ErrorData>;

	// Language module interface which should be implemented by user !
	class ILanguageModule {
	protected:
		~ILanguageModule() = default;

	public:
		virtual InitResult Initialize(std::weak_ptr<IPlugifyProvider> provider, const IModule& module) = 0;
		virtual void Shutdown() = 0;
		virtual LoadResult OnPluginLoad(const IPlugin& plugin) = 0;
		virtual void OnPluginStart(const IPlugin& plugin) = 0;
		virtual void OnPluginEnd(const IPlugin& plugin) = 0;
		virtual void OnMethodExport(const IPlugin& plugin) = 0;
	};
}
