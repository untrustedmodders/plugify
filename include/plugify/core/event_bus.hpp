#pragma once

#include <any>
#include <typeindex>
#include <functional>

namespace plugify {
    // Event system interface
    template<typename... Args>
    using EventHandler = std::function<void(Args...)>;

    class IEventBus {
    public:
        virtual ~IEventBus() = default;

        template<typename EventType>
        void Subscribe(EventHandler<const EventType&> handler);

        template<typename EventType>
        void Publish(const EventType& event);

        virtual void SubscribeRaw(std::type_index type, std::any handler) = 0;
        virtual void PublishRaw(std::type_index type, std::any event) = 0;
    };
}