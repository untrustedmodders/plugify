#include "core/glaze_manifest_parser.hpp"
#include "core/glaze_metadata.hpp"

using namespace plugify;

Result<ManifestPtr> ParsePlugin(
    const std::string& content,
    const std::filesystem::path& path
) {
    auto parsed = glz::read_jsonc<std::shared_ptr<PluginManifest>>(content);
    if (!parsed) {
        return plg::unexpected(glz::format_error(parsed.error(), content));
    }
    auto& manifest = parsed.value();
    manifest->type = PackageType::Plugin;
    manifest->path = path;
    return std::static_pointer_cast<PackageManifest>(std::move(manifest));
}

Result<ManifestPtr> ParseModule(
    const std::string& content,
    const std::filesystem::path& path
) {
    auto parsed = glz::read_jsonc<std::shared_ptr<ModuleManifest>>(content);
    if (!parsed) {
        return plg::unexpected(glz::format_error(parsed.error(), content));
    }
    auto& manifest = parsed.value();
    manifest->type = PackageType::Module;
    manifest->path = path;
    return std::static_pointer_cast<PackageManifest>(std::move(manifest));
}

Result<ManifestPtr> GlazeManifestParser::Parse(
    const std::string& content,
    const std::filesystem::path& path
) {
    static std::unordered_map<std::string, PackageType, plg::case_insensitive_hash, plg::case_insensitive_equal> manifests = {
        { ".pplugin", PackageType::Plugin },
        { ".pmodule", PackageType::Module }
    };

    auto ext = path.extension().string();
    auto it = manifests.find(ext);
    if (it == manifests.end()) {
        return plg::unexpected(std::format("Unknown manifest file extension: \"{}\"", ext));
    }
    switch (it->second) {
        case PackageType::Plugin:
            return ParsePlugin(content, path);
        case PackageType::Module:
            return ParseModule(content, path);
        default:
            PL_ASSERT(false && "Unsupported package type");
            return plg::unexpected("Unsupported package type");
    }
}
