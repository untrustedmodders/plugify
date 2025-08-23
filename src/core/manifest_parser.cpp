#include "plugify/core/manifest_parser.hpp"

#include <glaze/glaze.hpp>

using namespace plugify;

static std::unordered_map<std::string, PackageType, plg::case_insensitive_hash, plg::case_insensitive_equal> manifests = {
    {Plugin::kFileExtension, PackageType::Plugin},
    {Module::kFileExtension, PackageType::Module}
};

Result<ManifestPtr>
GlazeManifestParser::Parse(const std::string& content, const std::filesystem::path& path) {
    auto parsed = glz::read_jsonc<ManifestPtr>(content);
    if (!parsed) {
        return plg::unexpected(glz::format_error(parsed.error(), content));
    }

    auto& manifest = *parsed;

    if (!manifest->location) {
        manifest->location = path.parent_path();
    }

    if (!manifest->type || manifest->type == PackageType::Auto) {
        auto ext = path.extension().string();
        auto it = manifests.find(ext);
        if (it == manifests.end()) {
            return plg::unexpected(std::format("Unknown manifest file extension: \"{}\"", ext));
        }
        manifest->type = it->second;
    }

    return manifest;
}