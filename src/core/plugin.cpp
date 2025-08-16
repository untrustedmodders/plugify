#include "plugin.hpp"
#include "module.hpp"
#include <plugify/api/plugin.hpp>
#include <plugify/api/provider.hpp>

using namespace plugify;

Plugin::Plugin(UniqueId id, BasePaths paths, std::unique_ptr<Manifest> manifest)
	: _id{id}, _paths{std::move(paths)}, _manifest{static_unique_cast<PluginManifest>(std::move(manifest))} {
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

void Plugin::SetError(std::string error) {
	_error = std::move(error);
	_state = PluginState::Error;
	PL_LOG_ERROR("Plugin '{}': {}", GetName(), GetError());
}
