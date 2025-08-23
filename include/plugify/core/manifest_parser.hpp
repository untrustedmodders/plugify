#pragma once

#include <string>
#include <filesystem>

#include "plugify/core/types.hpp"
#include "plugify/core/manifest.hpp"

namespace plugify {
    /**
     * @brief Interface for parsing manifest files
     */
    class IManifestParser {
    public:
        virtual ~IManifestParser() = default;

        /**
         * @brief Parse the manifest content from a string
         * @param content The content of the manifest file as a string
         * @param path The filesystem path to the manifest file (for error reporting)
         * @return A Result containing the parsed Manifest or an error
         */
        virtual Result<ManifestPtr> Parse(const std::string& content, const std::filesystem::path& path) = 0;
    };

    /**
     * @brief A manifest parser using the Glaze library
     */
    class GlazeManifestParser : public IManifestParser {
    public:
        /**
         * @brief Parse the manifest content from a string
         * @param content The content of the manifest file as a string
         * @param path The filesystem path to the manifest file (for error reporting)
         * @return A Result containing the parsed Manifest or an error
         */
        Result<ManifestPtr> Parse(const std::string& content, const std::filesystem::path& path) override;
    };
}