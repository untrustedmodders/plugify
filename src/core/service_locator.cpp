#include "plugify/core/service_locator.hpp"

using namespace plugify;

struct ServiceLocator::Impl {
    std::unordered_map<std::type_index, std::shared_ptr<void>> services;
};

ServiceLocator::ServiceLocator() : _impl(std::make_unique<Impl>()) {}
ServiceLocator::~ServiceLocator() = default;
ServiceLocator::ServiceLocator(ServiceLocator&&) noexcept = default;
ServiceLocator& ServiceLocator::operator=(ServiceLocator&&) noexcept = default;

void ServiceLocator::RegisterInternal(std::type_index type, std::shared_ptr<void> service) {
    _impl->services[type] = std::move(service);
}

std::shared_ptr<void> ServiceLocator::GetInternal(std::type_index type) const {
    auto it = _impl->services.find(type);
    if (it != _impl->services.end()) {
        return it->second;
    }
    throw std::runtime_error("Service not found");
}

bool ServiceLocator::HasInternal(std::type_index type) const {
    return _impl->services.contains(type);
}

void ServiceLocator::Clear() {
    _impl->services.clear();
}