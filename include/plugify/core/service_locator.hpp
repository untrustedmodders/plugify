#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace plugify {
    class ServiceLocator {
    public:
        // Register service with type safety
        template<typename Interface, typename Implementation = Interface>
        void Register(std::shared_ptr<Implementation> service)
            requires(std::is_base_of_v<Interface, Implementation>) {
            _services[std::type_index(typeid(Interface))] = std::move(service);
        }

        // Get service with automatic casting
        template<typename Interface>
        [[nodiscard]] std::shared_ptr<Interface> Get() const {
            auto it = _services.find(std::type_index(typeid(Interface)));
            if (it != _services.end()) {
                return std::static_pointer_cast<Interface>(it->second);
            }
            throw std::runtime_error(std::format("Service not found: {}", typeid(Interface).name()));
        }

        // Check if service exists
        template<typename Interface>
        [[nodiscard]] bool Has() const {
            return _services.contains(std::type_index(typeid(Interface)));
        }

        // Clear all services
        void Clear() {
            _services.clear();
        }

    private:
        std::unordered_map<std::type_index, std::shared_ptr<void>> _services;
    };

    /*template<typename... Services>
    class ServiceContext {
        std::tuple<std::shared_ptr<Services>...> _services;

    public:
        explicit ServiceContext(const ServiceLocator& locator)
            : _services(locator.Get<Services>()...) {}

        template<typename T>
        std::shared_ptr<T> Get() const {
            return std::get<std::shared_ptr<T>>(_services);
        }

        bool IsValid() const {
            return (... && std::get<std::shared_ptr<Services>>(_services));
        }

        template<typename Func>
        auto Apply(Func&& func) const {
            return std::apply(std::forward<Func>(func), _services);
        }
    };*/
}