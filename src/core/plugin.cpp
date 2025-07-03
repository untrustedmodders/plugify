#include "plugin.hpp"
#include "module.hpp"
#include <plugify/package.hpp>
#include <plugify/plugify_provider.hpp>
#include <plugify/plugin.hpp>

using namespace plugify;

Plugin::Plugin(UniqueId id, const LocalPackage& package, const BasePaths& paths) : _id{id}, _name{package.name}, _descriptor{std::static_pointer_cast<PluginDescriptor>(package.descriptor)},
																				   _configsDir{paths.configs / _name}, _dataDir{paths.data / _name}, _logsDir{paths.logs / _name} {
	PL_ASSERT(package.type == PackageType::Plugin && "Invalid package type for plugin ctor");
	PL_ASSERT(package.path.has_parent_path() && "Package path doesn't contain parent path");
	_baseDir = package.path.parent_path();
}

Plugin::Plugin(Plugin&& plugin) noexcept {
	*this = std::move(plugin);
}

bool Plugin::Initialize(const std::shared_ptr<IPlugifyProvider>& provider) {
	PL_ASSERT(GetState() != PluginState::Loaded && "Plugin already was initialized");

	auto isRegularFile = [](const fs::path& path) {
		std::error_code ec;
		return fs::exists(path, ec) && fs::is_regular_file(path, ec);
	};

	std::error_code ec;
	fs::path baseDir = provider->GetBaseDir();

	if (const auto& resourceDirectoriesSettings = _descriptor->resourceDirectories) {
		for (const auto& rawPath : *resourceDirectoriesSettings) {
			fs::path resourceDirectory = fs::absolute(_baseDir / rawPath, ec);
			if (ec) {
				SetError(std::format("Failed to get resource directory path '{}' - {}", rawPath, ec.message()));
				return false;
			}
			for (const auto& entry : fs::recursive_directory_iterator(resourceDirectory, ec)) {
				if (entry.is_regular_file(ec)) {
					fs::path relPath = fs::relative(entry.path(), _baseDir, ec);
					fs::path absPath = baseDir / relPath;

					if (!isRegularFile(absPath)) {
						absPath = entry.path();
					}

					_resources.try_emplace(std::move(relPath), std::move(absPath));
				}
			}
		}
	}

	return true;
}

void Plugin::Terminate() {
	SetUnloaded();
}

std::optional<fs::path_view> Plugin::FindResource(const fs::path& path) const {
	auto it = _resources.find(path);
	if (it != _resources.end())
		return std::get<fs::path>(*it).native();

	return std::nullopt;
}

void Plugin::SetError(std::string error) {
	_error = std::make_unique<std::string>(std::move(error));
	_state = PluginState::Error;
	PL_LOG_ERROR("Plugin '{}': {}", _name, *_error);
}

Plugin& Plugin::operator=(Plugin&& other) noexcept {
	_module = other._module;
	_state = other._state;
	_table = other._table;
	_id = other._id;
	_data = other._data;

	_name = std::move(other._name);
	_baseDir = std::move(other._baseDir);
	_configsDir = std::move(other._configsDir);
	_dataDir = std::move(other._dataDir);
	_logsDir = std::move(other._logsDir);
	_methods = std::move(other._methods);
	_descriptor = std::move(other._descriptor);
	_resources = std::move(other._resources);
	_error = std::move(other._error);
	return *this;
}