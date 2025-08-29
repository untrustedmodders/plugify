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

const std::filesystem::path& Provider::GetExtensionsDir() const noexcept {
    return _impl->context->GetConfig().paths.extensionsDir;
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

bool Provider::IsExtensionLoaded(std::string_view name, std::optional<Constraint> version) const noexcept {
    return _impl->manager->IsExtensionLoaded(name, std::move(version));
}

std::shared_ptr<Extension> Provider::FindExtension(std::string_view name) const {
    return _impl->manager->FindExtension(name);
}

std::vector<std::shared_ptr<Extension>> Provider::GetExtensions() const {
    return _impl->manager->GetExtensions();
}

bool Provider::operator==(const Provider& other) const noexcept = default;

auto Provider::operator<=>(const Provider& other) const noexcept = default;

std::shared_ptr<Context> Provider::GetContext() const noexcept {
    return _impl->context;
}
