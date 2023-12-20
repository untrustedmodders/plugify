#pragma once

namespace wizard {
    class LanguageModule {
    public:
        /**
         * @brief Called when the module is initialized by the core.
         * @return Returns false on error.
         */
        virtual bool OnInit() = 0;

        /**
         * @brief Called when a module is destroy. This function will be not called if false is returned by Load().
         */
        virtual void OnDestroy() = 0;

        //
        virtual void OnNativeAdded(/*data*/) = 0;

        //
        virtual void* GetExportedNative(/*data*/) = 0;

        //
        virtual bool LoadPlugin(/*data*/) = 0;
    };
}
