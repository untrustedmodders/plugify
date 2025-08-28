#include <utility>

#include "plugify/core/provider.hpp"
#include "plugify/core/context.hpp"
#include "plugify/core/manager.hpp"

using namespace plugify;

struct Provider::Impl {
    std::shared_ptr<Context> context;
    std::shared_ptr<Manager> manager;
};

Provider::Provider(std::shared_ptr<Context> context, std::shared_ptr<Manager> manager)
    : _impl(std::make_unique<Impl>(std::move(context), std::move(manager))) {
}

Provider::~Provider() = default;

/*Provider::Provider(const Provider& other)
    : _impl(std::make_unique<Impl>(*other._impl)) {}

Provider::Provider(Provider&& other) noexcept = default;

Provider& Provider::operator=(const Provider& other) {
    if (this != &other) {
        _impl = std::make_unique<Impl>(*other._impl);
    }
    return *this;
}
Provider& Provider::operator=(Provider&& other) noexcept = default;*/

void Provider::Log(std::string_view msg, Severity severity, const std::source_location& loc) const {
    if (auto logger = _impl->context->GetService<ILogger>()) {
        logger->Log(msg, severity, loc);
    }
}

bool Provider::IsPreferOwnSymbols() const noexcept {
    return _impl->context->GetConfig().loading.preferOwnSymbols;
}

const std::filesystem::path& Provider::GetBaseDir() const noexcept {
    return _impl->context->GetConfig().paths.baseDir;
}

const std::filesystem::path& Provider::GetPluginsDir() const noexcept {
    return _impl->context->GetConfig().paths.pluginsDir;
}

const std::filesystem::path& Provider::GetConfigsDir() const noexcept {
    return _impl->context->GetConfig().paths.configsDir;
}

const std::filesystem::path& Provider::GetDataDir() const noexcept {
    return _impl->context->GetConfig().paths.dataDir;
}

const std::filesystem::path& Provider::GetLogsDir() const noexcept {
    return _impl->context->GetConfig().paths.logsDir;
}

const std::filesystem::path& Provider::GetCacheDir() const noexcept {
    return _impl->context->GetConfig().paths.cacheDir;
}

bool Provider::IsPluginLoaded(std::string_view name, std::optional<Constraint> version) const noexcept {
    return _impl->manager->IsPluginLoaded(name, version);
}

std::shared_ptr<Plugin> Provider::FindPlugin(std::string_view name) const {
    return _impl->manager->FindPlugin(name);
}

std::vector<std::shared_ptr<Plugin>> Provider::GetPlugins() const {
    return _impl->manager->GetPlugins();
}

bool Provider::IsModuleLoaded(std::string_view name, std::optional<Constraint> version) const noexcept {
    return _impl->manager->IsModuleLoaded(name, version);
}

std::shared_ptr<Module> Provider::FindModule(std::string_view name) const {
    return _impl->manager->FindModule(name);
}

std::vector<std::shared_ptr<Module>> Provider::GetModules() const {
    return _impl->manager->GetModules();
}

std::shared_ptr<Context> Provider::GetContext() const noexcept {
    return _impl->context;
}
