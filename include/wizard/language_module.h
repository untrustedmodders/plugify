#pragma once

namespace wizard {
    // Language module interface which should be implemented by user !
    class ILanguageModule {
    public:
        virtual ~ILanguageModule() = default;

        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void* GetMethod(const std::string& name) = 0;
        virtual void OnNativeAdded(/*data*/) = 0;
        virtual bool OnLoadPlugin(/*data*/) = 0;
    };
}
