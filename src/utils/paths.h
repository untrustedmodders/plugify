#pragma once

namespace wizard {
    /**
     * Path helpers for retrieving plugin dir, modules dir, etc.
     */
    class WIZARD_API Paths {
    public:
        Paths() = delete;

        /**
         * Returns the modules directory of the core.
         *
         * @return Modules directory.
         */
        static const fs::path& ModulesDir();

        /**
         * Returns the plugins directory of the core.
         *
         * @return Plugins directory.
         */
        static const fs::path& PluginsDir();

        /**
         * Gets the file extention in the lowercase format.
         * @param filepath The path to the file.
         * @return The string extension.
         */
        static std::string GetExtension(const fs::path& filepath);
    };
}