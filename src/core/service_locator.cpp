#include "plugify/service_locator.hpp"

using namespace plugify;

// ============================================
// Implementation struct
// ============================================
struct ServiceLocator::Impl {
	struct ServiceDescriptor {
		std::function<std::shared_ptr<void>()> factory;
		ServiceLifetime lifetime;
		std::shared_ptr<void> singleton;  // Cached for singleton lifetime
	};

	mutable std::shared_mutex mutex;
	std::unordered_map<std::type_index, ServiceDescriptor> services;

	// Scoped instances (for scoped lifetime)
	mutable std::unordered_map<std::type_index, std::shared_ptr<void>> scopedInstances;

	// Methods
	void RegisterInstance(std::type_index type, std::shared_ptr<void> instance) {
		std::unique_lock lock(mutex);
		services[type] = { .factory = [instance]() { return instance; },
			               .lifetime = ServiceLifetime::Singleton,
			               .singleton = instance };
	}

	void RegisterFactory(
	    std::type_index type,
	    std::function<std::shared_ptr<void>()> factory,
	    ServiceLifetime lifetime
	) {
		std::unique_lock lock(mutex);

		ServiceDescriptor descriptor{ .factory = std::move(factory),
			                          .lifetime = lifetime,
			                          .singleton = nullptr };

		// For singleton, create immediately
		if (lifetime == ServiceLifetime::Singleton) {
			descriptor.singleton = descriptor.factory();
		}

		services[type] = std::move(descriptor);
	}

	std::shared_ptr<void> Resolve(std::type_index type) const {
		std::shared_lock lock(mutex);

		auto it = services.find(type);
		if (it == services.end()) {
			throw std::runtime_error(std::format("Service not registered: {}", type.name()));
		}

		const auto& descriptor = it->second;

		switch (descriptor.lifetime) {
			case ServiceLifetime::Singleton:
				return descriptor.singleton;

			case ServiceLifetime::Scoped: {
				auto scopedIt = scopedInstances.find(type);
				if (scopedIt != scopedInstances.end()) {
					return scopedIt->second;
				}

				// Need to upgrade to write lock
				lock.unlock();
				std::unique_lock writeLock(mutex);

				// Check again in case another thread created it
				scopedIt = scopedInstances.find(type);
				if (scopedIt != scopedInstances.end()) {
					return scopedIt->second;
				}

				// Create new scoped instance
				auto instance = descriptor.factory();
				scopedInstances[type] = instance;
				return instance;
			}

			case ServiceLifetime::Transient:
			default:
				return descriptor.factory();
		}
	}

	std::shared_ptr<void> TryResolve(std::type_index type) const noexcept {
		try {
			return Resolve(type);
		} catch (...) {
			return nullptr;
		}
	}

	bool IsRegistered(std::type_index type) const {
		std::shared_lock lock(mutex);
		return services.contains(type);
	}

	void BeginScope() {
		std::unique_lock lock(mutex);
		scopedInstances.clear();
	}

	void EndScope() {
		std::unique_lock lock(mutex);
		scopedInstances.clear();
	}

	void Clear() {
		std::unique_lock lock(mutex);
		services.clear();
		scopedInstances.clear();
	}

	size_t Count() const {
		std::shared_lock lock(mutex);
		return services.size();
	}
};

// ============================================
// ServiceLocator implementation
// ============================================
ServiceLocator::ServiceLocator()
    : _impl(std::make_unique<Impl>()) {
}

ServiceLocator::~ServiceLocator() = default;

ServiceLocator::ServiceLocator(ServiceLocator&&) noexcept = default;
ServiceLocator& ServiceLocator::operator=(ServiceLocator&&) noexcept = default;

void ServiceLocator::RegisterInstanceInternal(std::type_index type, std::shared_ptr<void> instance) {
	_impl->RegisterInstance(type, std::move(instance));
}

void ServiceLocator::RegisterFactoryInternal(
    std::type_index type,
    std::function<std::shared_ptr<void>()> factory,
    ServiceLifetime lifetime
) {
	_impl->RegisterFactory(type, std::move(factory), lifetime);
}

std::shared_ptr<void> ServiceLocator::ResolveInternal(std::type_index type) const {
	return _impl->Resolve(type);
}

std::shared_ptr<void> ServiceLocator::TryResolveInternal(std::type_index type) const noexcept {
	return _impl->TryResolve(type);
}

bool ServiceLocator::IsRegisteredInternal(std::type_index type) const {
	return _impl->IsRegistered(type);
}

void ServiceLocator::BeginScope() {
	_impl->BeginScope();
}

void ServiceLocator::EndScope() {
	_impl->EndScope();
}

void ServiceLocator::Clear() {
	_impl->Clear();
}

size_t ServiceLocator::Count() const {
	return _impl->Count();
}

ServiceLocator::ServiceBuilder ServiceLocator::Services() {
	return ServiceBuilder(*this);
}

// ============================================
// ServiceBuilder implementation
// ============================================
ServiceLocator::ServiceBuilder::ServiceBuilder(ServiceLocator& locator)
    : _locator(locator) {
}

ServiceLocator::ServiceBuilder::~ServiceBuilder() = default;

// ============================================
// ScopedServiceLocator implementation
// ============================================
ScopedServiceLocator::ScopedServiceLocator(ServiceLocator& locator)
    : _locator(locator) {
	_locator.BeginScope();
}

ScopedServiceLocator::~ScopedServiceLocator() {
	_locator.EndScope();
}