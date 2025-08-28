#include "core/glaze_manifest_parser.hpp"
#include "core/glaze_metadata.hpp"

using namespace plugify;

Result<PackageManifest> GlazeManifestParser::Parse(
    const std::string& content,
    PackageType type
) {
    auto parsed = glz::read_jsonc<PackageManifest>(content);
    if (!parsed) {
        return plg::unexpected(glz::format_error(parsed.error(), content));
    }
    return *parsed;
}
