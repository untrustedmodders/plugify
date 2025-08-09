#pragma once

#include <any>
#include <typeindex>
#include <functional>

namespace plugify {
    // Event bus for decoupled communication
    class IEventBus {
    public:
        virtual ~IEventBus() = default;

        using EventHandler = std::function<void(const std::any&)>;
        using SubscriptionId = size_t;

        virtual SubscriptionId Subscribe(std::string_view eventType, EventHandler handler) = 0;
        virtual void Unsubscribe(SubscriptionId id) = 0;
        virtual void Publish(std::string_view eventType, std::any data) = 0;

        // Type-safe publish/subscribe
        template<typename T>
        SubscriptionId Subscribe(std::function<void(const T&)> handler) {
            return Subscribe(typeid(T).name(), [handler](const std::any& data) {
                if (auto* ptr = std::any_cast<T>(&data)) {
                    handler(*ptr);
                }
            });
        }

        template<typename T>
        void Publish(T&& data) {
            Publish(typeid(T).name(), std::any(std::forward<T>(data)));
        }
    };
}