#pragma once

namespace wizard {
    class LanguageModuleDescriptor;

    /**
     * Information about a language module.
     */
    class WIZARD_API ILanguageModule {
    public:
        /* Virtual destructor */
        virtual ~ILanguageModule() = default;

        /**
         * Called when the module is initialized by the core.
         * @return Returns false on error.
         */
        virtual bool Initialize() = 0;

        /**
         * Called when a module is destroy. This function will be not called if false is returned by Load().
         */
        virtual void Shutdown() = 0;

        /** */
        virtual void* GetMethod(const std::string& name) = 0;

        /** */
        virtual void OnNativeAdded(/*data*/) = 0;

        /** */
        virtual bool OnLoadPlugin(/*data*/) = 0;

        /**
         * Gets the module name.
         *
         * @return Name of the module.
         */
        virtual const std::string& GetName() const = 0;

        /**
         * Returns the module friendly name if available; otherwise, returns the same name as GetName().
         */
        virtual const std::string& GetFriendlyName() const = 0;

        /**
         * Gets a filesystem path to the module's descriptor.
         *
         * @return Filesystem path to the module's descriptor.
         */
        virtual const fs::path& GetDescriptorFilePath() const = 0;

        /**
         * Gets a filesystem path to the module's directory.
         *
         * @return Filesystem path to the module's base directory.
         */
        virtual fs::path GetBaseDir() const = 0;

        /**
         * Gets a filesystem path to the module's binaries directory.
         *
         * @return Filesystem path to the module's binaries directory.
         */
        virtual fs::path GetBinariesDir() const = 0;

        /**
         * Gets the module's descriptor.
         *
         * @return Reference to the module's descriptor.
         */
        virtual const LanguageModuleDescriptor& GetDescriptor() const = 0;
    };
}
