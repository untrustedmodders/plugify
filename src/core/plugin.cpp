#include "plugin.hpp"
#include "module.hpp"
#include <plugify/api/plugify_provider.hpp>
#include <plugify/api/plugin.hpp>

using namespace plugify;

Plugin::Plugin(UniqueId id, std::string_view name, std::unique_ptr<Manifest> manifest, const BasePaths& paths)
	: _id{id}
	, _name{name}
	, _dirs{paths}
	, _manifest{std::unique_ptr<PluginManifest>(static_cast<PluginManifest*>(manifest.release()))} {
	PL_ASSERT(type == PackageType::Plugin && "Invalid package type for plugin ctor");
	PL_ASSERT(path.has_parent_path() && "Package path doesn't contain parent path");
	_baseDir = manifest->path.parent_path();
}

Plugin::Plugin(Plugin&& plugin) noexcept {
	*this = std::move(plugin);
}

bool Plugin::Initialize(const std::shared_ptr<IPlugifyProvider>&) {
	PL_ASSERT(GetState() != PluginState::Loaded && "Plugin already was initialized");
	return true;
}

void Plugin::Terminate() {
	SetUnloaded();
}

void Plugin::SetError(std::string error) {
	_error = std::move(error);
	_state = PluginState::Error;
	PL_LOG_ERROR("Plugin '{}': {}", _name, _error);
}
