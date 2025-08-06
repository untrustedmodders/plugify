#include "plugin.hpp"
#include "module.hpp"
#include <plugify/package.hpp>
#include <plugify/plugify_provider.hpp>
#include <plugify/plugin.hpp>

using namespace plugify;

Plugin::Plugin(UniqueId id, const LocalPackage& package, const BasePaths& paths)
	: _id{id}
	, _name{package.name}
	, _configsDir{paths.configs / package.name}
	, _dataDir{paths.data / package.name}
	, _logsDir{paths.logs / package.name}
	, _descriptor{std::static_pointer_cast<PluginDescriptor>(package.descriptor)} {
	PL_ASSERT(package.type == PackageType::Plugin && "Invalid package type for plugin ctor");
	PL_ASSERT(package.path.has_parent_path() && "Package path doesn't contain parent path");
	_baseDir = package.path.parent_path();
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
	_error = std::make_unique<std::string>(std::move(error));
	_state = PluginState::Error;
	PL_LOG_ERROR("Plugin '{}': {}", _name, *_error);
}
