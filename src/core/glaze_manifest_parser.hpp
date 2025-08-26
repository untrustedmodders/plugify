#include once

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
         * @param path The filesystem path to the manifest file (for error reporting)
         * @return A Result containing the parsed Manifest or an error
         */
        Result<ManifestPtr> Parse(const std::string& content, const std::filesystem::path& path) override;
    };
}