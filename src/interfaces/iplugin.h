#pragma once

namespace wizard {
    class PluginDescriptor;

    /**
     * Information about a plugin.
     */
    class IPlugin {
    public:
        /* Virtual destructor */
        virtual ~IPlugin() = default;

        /**
         * Gets the plugin id.
         *
         * @return Id of the plugin.
         */
        virtual uint64_t GetId() const = 0;

        /**
         * Gets the plugin name.
         *
         * @return Name of the plugin.
         */
        virtual const std::string& GetName() const = 0;

        /**
         * Returns the plugin friendly name if available; otherwise, returns the same name as GetName().
         */
        virtual const std::string& GetFriendlyName() const = 0;

        /**
         * Gets a filesystem path to the plugin's descriptor.
         *
         * @return Filesystem path to the plugin's descriptor.
         */
        virtual const fs::path& GetDescriptorFilePath() const = 0;

        /**
         * Gets a filesystem path to the plugin's directory.
         *
         * @return Filesystem path to the plugin's base directory.
         */
        virtual fs::path GetBaseDir() const = 0;

        /**
         * Gets a filesystem path to the plugin's content directory.
         *
         * @return Filesystem path to the plugin's content directory.
         */
        virtual fs::path GetContentDir() const = 0;

        /**
         * Gets the virtual root path for assets.
         *
         * @return The mounted root path for assets in this plugin's content folder; typically /PluginName/.
         */
        virtual fs::path GetMountedAssetPath() const = 0;

        /**
         * Gets the plugin's descriptor.
         *
         * @return Reference to the plugin's descriptor.
         */
        virtual const PluginDescriptor& GetDescriptor() const = 0;
    };
}