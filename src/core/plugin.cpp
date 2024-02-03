#include "plugin.h"
#include "module.h"
#include <plugify/plugin.h>
#include <plugify/package.h>

using namespace plugify;

Plugin::Plugin(UniqueId id, const LocalPackage& package) : IPlugin(*this), _id{id}, _name{package.name}, _descriptor{std::static_pointer_cast<PluginDescriptor>(package.descriptor)} {
	PL_ASSERT(package.type == "plugin", "Invalid package type for plugin ctor");
	PL_ASSERT(package.path.has_parent_path(), "Package path doesn't contain parent path");
	_baseDir = package.path.parent_path();
	_contentDir = _baseDir / "content";
	_filePath /= _baseDir / _descriptor->assemblyPath;
}

void Plugin::SetError(std::string error) {
	_error = std::move(error);
	_state = PluginState::Error;
	PL_LOG_ERROR("Plugin '{}': {}", _name, _error);
}