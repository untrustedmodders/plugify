#include <utility>

#include "plugify/provider.hpp"
#include "plugify/config.hpp"
#include "plugify/manager.hpp"

using namespace plugify;

struct Provider::Impl {
    const ServiceLocator& services;
    const Config& config;
    const Manager& manager;
};

Provider::Provider(const ServiceLocator& services, const Config& config, const Manager& manager)
    : _impl(std::make_unique<Impl>(services, config, manager)) {
}

Provider::~Provider() = default;

Provider::Provider(const Provider& other)
    : _impl(std::make_unique<Impl>(*other._impl)) {}

Provider::Provider(Provider&& other) noexcept = default;

Provider& Provider::operator=(const Provider& other) {
    if (this != &other) {
        _impl = std::make_unique<Impl>(*other._impl);
    }
    return *this;
}
Provider& Provider::operator=(Provider&& other) noexcept = default;

void Provider::Log(std::string_view msg, Severity severity, const std::source_location& loc) const {
    if (auto logger = _impl->services.Resolve<ILogger>()) {
        logger->Log(msg, severity, loc);
    }
}

bool Provider::IsPreferOwnSymbols() const noexcept {
    return _impl->config.loading.preferOwnSymbols;
}

const std::filesystem::path& Provider::GetBaseDir() const noexcept {
    return _impl->config.paths.baseDir;
}

const std::filesystem::path& Provider::GetExtensionsDir() const noexcept {
    return _impl->config.paths.extensionsDir;
}

const std::filesystem::path& Provider::GetConfigsDir() const noexcept {
    return _impl->config.paths.configsDir;
}

const std::filesystem::path& Provider::GetDataDir() const noexcept {
    return _impl->config.paths.dataDir;
}

const std::filesystem::path& Provider::GetLogsDir() const noexcept {
    return _impl->config.paths.logsDir;
}

const std::filesystem::path& Provider::GetCacheDir() const noexcept {
    return _impl->config.paths.cacheDir;
}

bool Provider::IsExtensionLoaded(std::string_view name, std::optional<Constraint> constraint) const noexcept {
    return _impl->manager.IsExtensionLoaded(name, std::move(constraint));
}

const Extension* Provider::FindExtension(std::string_view name) const noexcept {
    return _impl->manager.FindExtension(name);
}

const Extension* Provider::FindExtension(UniqueId id) const noexcept {
    return _impl->manager.FindExtension(id);
}

std::vector<const Extension*> Provider::GetExtensions() const {
    return _impl->manager.GetExtensions();
}

bool Provider::operator==(const Provider& other) const noexcept = default;

auto Provider::operator<=>(const Provider& other) const noexcept = default;

const ServiceLocator& Provider::GetServices() const noexcept {
    return _impl->services;
}
