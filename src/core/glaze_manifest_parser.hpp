#pragma once

#include "plugify/core/manifest_parser.hpp"
#include "core/glaze_metadata.hpp"

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
        Result<Manifest> Parse(const std::string& content, [[maybe_unused]] ExtensionType type) override  {
            auto parsed = glz::read_jsonc<Manifest>(content);
            if (!parsed) {
                return plg::unexpected(glz::format_error(parsed.error(), content));
            }
            return *parsed;
        }
    };
}