#pragma once

#include "load_result.h"
#include <string>

namespace wizard {
    class IPlugin;

    // Language module interface which should be implemented by user !
    class ILanguageModule {
    protected:
        ~ILanguageModule() = default;

    public:
        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void OnNativeAdded(/*data*/) = 0;
        virtual LoadResult OnPluginLoad(const IPlugin& plugin) = 0;
        virtual void OnPluginStart(const IPlugin& plugin) = 0;
        virtual void OnPluginEnd(const IPlugin& plugin) = 0;
    };
}
