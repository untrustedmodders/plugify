#pragma once

#include "load_result.h"

namespace wizard {
    class IPlugin;

    // Language module interface which should be implemented by user !
    class ILanguageModule {
    protected:
        ~ILanguageModule() = default;

    public:
        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void* GetMethod(const std::string& name) = 0;
        virtual void OnNativeAdded(/*data*/) = 0;
        virtual LoadResult OnLoadPlugin(const IPlugin& plugin) = 0;
        virtual void OnStartPlugin(const IPlugin& plugin) = 0;
    };
}
