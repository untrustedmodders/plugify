#include <utility>

#include "glaze/core/context.hpp"
#include "plugify/core/manager.hpp"
#include "plugify/core/plugify.hpp"
#include "plugify/core/provider.hpp"

using namespace plugify;

struct Provider::Impl {
    std::shared_ptr<ILogger> logger;
    std::shared_ptr<IFileSystem> fileSystem;

    Impl(std::shared_ptr<Context> context) {

    }

    std::filesystem::path GetFullPath(const std::filesystem::path& relativePath) const {
        if (relativePath.is_absolute()) {
            return relativePath;
        }
        return paths.baseDir / relativePath;
    }
};

Provider::Provider(std::shared_ptr<Context> context)
    : _impl(std::make_unique<Impl>(context)) {
}

Provider::~Provider() = default;

void Provider::Log(std::string_view msg, Severity severity) const {
    if (auto logger = _impl->context->GetLogger()) {
        logger->Log(msg, severity);
    }
}

void Provider::Log(Severity severity, std::string_view msg, const std::source_location& loc) const {
    if (auto logger = _impl->context->GetLogger()) {
        logger->Log(msg, severity, loc);
    }
}

const std::filesystem::path& Provider::GetBaseDir() const noexcept {
    if (auto config = _impl->context->GetConfig()) {
        return config
    }
}

const std::filesystem::path& Provider::GetPluginsDir() const noexcept {
    static std::filesystem::path fullPath = _impl->GetFullPath(_impl->context>GetConfig().pluginsDir);
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

std::shared_ptr<Context> Provider::GetContext() const noexcept {
    return _impl->context;
}