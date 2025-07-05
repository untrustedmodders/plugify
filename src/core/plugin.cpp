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

bool Plugin::Initialize(const std::shared_ptr<IPlugifyProvider>& provider) {
	PL_ASSERT(GetState() != PluginState::Loaded && "Plugin already was initialized");

	std::error_code ec;

	fs::path baseDir = provider->GetBaseDir();

	if (const auto& resourceDirectoriesSettings = _descriptor->resourceDirectories) {
		for (const auto& rawPath : *resourceDirectoriesSettings) {
			fs::path resourceDirectory = _baseDir / rawPath;
			if (!fs::is_directory(resourceDirectory, ec)) {
				SetError(std::format("Resource directory '{}' not exists", resourceDirectory.string()));
				return false;
			}
			for (const auto& entry : fs::recursive_directory_iterator(resourceDirectory, ec)) {
				if (entry.is_regular_file(ec)) {
					fs::path relPath = fs::relative(entry.path(), _baseDir, ec);
					fs::path absPath = baseDir / relPath;

					if (!fs::is_regular_file(absPath, ec)) {
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
