#include "plugify/core/plugin.hpp"
#include "plugify/core/manifest.hpp"
#if 0
using namespace plugify;

// Plugin Implementation

struct Plugin::Impl {
	std::shared_ptr<PackageManifest> manifest;
};

Plugin::Plugin() : _impl(std::make_unique<Impl>()) {}
Plugin::~Plugin() = default;

Plugin::Plugin(const Plugin& other)
	: _impl(std::make_unique<Impl>(*other._impl)) {}

Plugin::Plugin(Plugin&& other) noexcept = default;

Plugin& Plugin::operator=(const Plugin& other) {
	if (this != &other) {
		_impl = std::make_unique<Impl>(*other._impl);
	}
	return *this;
}
Plugin& Plugin::operator=(Plugin&& other) noexcept = default;

std::string_view Plugin::GetId() const noexcept { return _impl->manifest->id; }
std::string_view Plugin::GetName() const noexcept { return _impl->manifest->name; }
PackageType Plugin::GetType() const noexcept { return _impl->manifest->type; }
Version Plugin::GetVersion() const noexcept { return _impl->manifest->version; }
std::string_view Plugin::GetDescription() const noexcept { return _impl->manifest->description.value_or({}); }
std::string_view Plugin::GetAuthor() const noexcept { return _impl->manifest->author.value_or({}); }
std::string_view Plugin::GetWebsite() const noexcept { return _impl->manifest->website.value_or({}); }
std::string_view Plugin::GetLicense() const noexcept { return _impl->manifest->license.value_or({}); }
const std::filesystem::path& Plugin::GetLocation() const noexcept { return _impl->manifest->location; }
std::span<const std::string> Plugin::GetPlatforms() const noexcept {
    return _impl->manifest->platforms.value_or({});
}
void Plugin::SetId(std::string_view id) noexcept { _impl->manifest->id = id; }
void Plugin::SetName(std::string_view name) noexcept { _impl->manifest->name = name; }
void Plugin::SetType(PackageType type) noexcept { _impl->manifest->type = type; }
void Plugin::SetVersion(Version version) noexcept { _impl->manifest->version = version; }
void Plugin::SetDescription(std::string_view description) noexcept { 
	_impl->manifest->description = description;
}
void Plugin::SetAuthor(std::string_view author) noexcept { _impl->manifest->author = author; }
void Plugin::SetWebsite(std::string_view website) noexcept { _impl->manifest->website = website; }
void Plugin::SetLicense(std::string_view license) noexcept { _impl->manifest->license = license; }
void Plugin::SetLocation(std::filesystem::path location) noexcept { 
	_impl->manifest->location = std::move(location); 
}
void Plugin::SetPlatforms(std::span<const std::string> platforms) noexcept {
	_impl->manifest->platforms = { platforms.begin(), platforms.end() };
}
void Plugin::SetDependencies(std::span<const Dependency> dependencies) noexcept {
	_impl->manifest->dependencies = { dependencies.begin(), dependencies.end() };
}
void Plugin::SetConflicts(std::span<const Conflict> conflicts) noexcept {
	_impl->manifest->conflicts = { conflicts.begin(), conflicts.end() };
}




















Plugin::Plugin(UniqueId id, BasePaths paths, std::shared_ptr<Manifest> manifest)
	: _id{id}, _paths{std::move(paths)}, _manifest{std::static_pointer_cast<PluginManifest>(std::move(manifest))} {
	PL_ASSERT(_manifest->type == PackageType::Plugin && "Invalid package type for plugin ctor");
	PL_ASSERT(_manifest->path.has_parent_path() && "Package path doesn't contain parent path");
}

Plugin::Plugin(Plugin&& plugin) noexcept {
	*this = std::move(plugin);
}

bool Plugin::Initialize(Plugify&) {
	PL_ASSERT(GetState() != PluginState::Loaded && "Plugin already was initialized");
	return true;
}

void Plugin::Terminate() {
	SetUnloaded();
}

void Plugin::SetError(std::string_view error) {
	PL_ASSERT(error.empty() && "Empty error string!");
	_error = std::move(error);
	_state = PluginState::Error;
	PL_LOG_ERROR("Plugin '{}': {}", GetName(), GetError());
}

#endif