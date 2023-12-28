#pragma once

#include <variant>
#include <string>
#include <vector>

namespace wizard {
    class IPlugin;
    class IModule;
    class IWizardProvider;

    struct ErrorData {
        std::string error;
    };

    struct InitResultData {};

    struct LoadResultData {
        std::vector<std::pair<std::string, void*>> methods;
    };

    using InitResult = std::variant<LoadResultData, ErrorData>;
    using LoadResult = std::variant<InitResultData, ErrorData>;
    
    // Language module interface which should be implemented by user !
    class ILanguageModule {
    protected:
        ~ILanguageModule() = default;

    public:
        virtual InitResult Initialize(std::weak_ptr<IWizardProvider> provider, const IModule& module) = 0;
        virtual void Shutdown() = 0;
        virtual void OnNativeAdded(/*data*/) = 0;
        virtual LoadResult OnPluginLoad(const IPlugin& plugin) = 0;
        virtual void OnPluginStart(const IPlugin& plugin) = 0;
        virtual void OnPluginEnd(const IPlugin& plugin) = 0;
    };
}
