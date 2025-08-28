#pragma once

#include "plugify/core/manifest_parser.hpp"

namespace plugify {
    /**
     * @brief A manifest parser using the Glaze library
     */
    class GlazeManifestParser : public IManifestParser {
    public:
        /**
         * @brief Parse the manifest content from a string
         * @param content The content of the manifest file as a string
         * @param type The manifest type
         * @return A Result containing the parsed Manifest or an error
         */
        Result<PackageManifest> Parse(const std::string& content, PackageType type) override;
    };
}