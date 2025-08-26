#include <utility>

#include "plugify/core/plugify.hpp"
#include "plugify/core/manager.hpp"
#include "plugify/core/provider.hpp"

using namespace plugify;

struct Provider::Impl {
    struct Paths {
        std::filesystem::path baseDir;
        std::filesystem::path pluginsDir;
        std::filesystem::path configsDir;
        std::filesystem::path dataDir;
        std::filesystem::path logsDir;
        std::filesystem::path cacheDir;
    } paths;
    std::shared_ptr<ILogger> logger;
    std::shared_ptr<IFileSystem> fileSystem;
    std::shared_ptr<IAssemblyLoader> assemblyLoader;
    std::shared_ptr<IConfigProvider> configProvider;

    Impl(std::weak_ptr<ServiceLocator> services) {
        // Cache frequently used services
        if (auto s = services.lock()) {
            logger = s->Get<ILogger>();
            fileSystem = s->Get<IFileSystem>();
            assemblyLoader = s->Get<IAssemblyLoader>();
            configProvider = s->Get<IConfigProvider>();

            // Load configuration settings
            LoadConfiguration();
        }
    }

    void LoadConfiguration() {
        // Continue loading other config values...
        LoadPathConfig(configProvider, "paths.baseDir", paths.baseDir);
        LoadPathConfig(configProvider, "paths.pluginsDir", paths.pluginsDir);
        LoadPathConfig(configProvider, "paths.configsDir", paths.configsDir);
        LoadPathConfig(configProvider, "paths.dataDir", paths.dataDir);
        LoadPathConfig(configProvider, "paths.logsDir", paths.logsDir);
        LoadPathConfig(configProvider, "paths.cacheDir", paths.cacheDir);

        // Ensure directories exist
        //CreateDirectories();
    }

    void LoadPathConfig(const std::shared_ptr<IConfigProvider>& provider,
                       std::string_view key,
                       std::filesystem::path& target) {
        if (auto result = provider->GetValue(key); result) {
            if (auto* path = std::any_cast<std::string>(&result.value())) {
                target = *path;
            }
        }
    }

    void CreateDirectories() {
        auto createDir = [&](const std::filesystem::path& dir) {
            if (!dir.empty() && !fileSystem->IsExists(dir)) {
                auto result = fileSystem->CreateDirectories(dir);
                if (!result) {
                    logger->Log(std::format("Failed to create directory '{}': {}", dir.string(), result.error()), Severity::Error);
                } else {
                    logger->Log(std::format("Created directory: {}", dir.string()), Severity::Info);
                }
            }
        };

        createDir(paths.baseDir);
        createDir(paths.baseDir / paths.pluginsDir);
        createDir(paths.baseDir / paths.configsDir);
        createDir(paths.baseDir / paths.dataDir);
        createDir(paths.baseDir / paths.logsDir);
        createDir(paths.baseDir / paths.cacheDir);
    }

    std::filesystem::path GetFullPath(const std::filesystem::path& relativePath) const {
        if (relativePath.is_absolute()) {
            return relativePath;
        }
        return paths.baseDir / relativePath;
    }
};

Provider::Provider(std::weak_ptr<ServiceLocator> services)
    : _impl(std::make_unique<Impl>(services)) {
}

Provider::~Provider() = default;

void Provider::Log(std::string_view msg, Severity severity) const {
    if (_impl->logger) {
        _impl->logger->Log(msg, severity);
    }
}

const std::filesystem::path& Provider::GetBaseDir() const noexcept {
    return _impl->paths.baseDir;
}

const std::filesystem::path& Provider::GetPluginsDir() const noexcept {
    static std::filesystem::path fullPath = _impl->GetFullPath(_impl->paths.pluginsDir);
    return fullPath;
}

const std::filesystem::path& Provider::GetConfigsDir() const noexcept {
    static std::filesystem::path fullPath = _impl->GetFullPath(_impl->paths.configsDir);
    return fullPath;
}

const std::filesystem::path& Provider::GetDataDir() const noexcept {
    static std::filesystem::path fullPath = _impl->GetFullPath(_impl->paths.dataDir);
    return fullPath;
}

const std::filesystem::path& Provider::GetLogsDir() const noexcept {
    static std::filesystem::path fullPath = _impl->GetFullPath(_impl->paths.logsDir);
    return fullPath;
}

const std::filesystem::path& Provider::GetCacheDir() const noexcept {
    static std::filesystem::path fullPath = _impl->GetFullPath(_impl->paths.cacheDir);
    return fullPath;
}
