#pragma once

#include <any>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace plugify {
    class ServiceLocator {
    public:
        template<typename Interface>
        void Register(std::shared_ptr<Interface> service) {
            _services[std::type_index(typeid(Interface))] = service;
        }

        template<typename Interface>
        std::shared_ptr<Interface> Get() const {
            auto it = _services.find(std::type_index(typeid(Interface)));
            if (it != _services.end()) {
                return std::any_cast<std::shared_ptr<Interface>>(it->second);
            }
            return nullptr;
        }

        template<typename Interface>
        bool Has() const {
            return _services.contains(std::type_index(typeid(Interface)));
        }

    private:
        std::unordered_map<std::type_index, std::any> _services;
    };
}