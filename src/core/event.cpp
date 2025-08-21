#include "plugify/core/event.hpp"

using namespace plugify;

HandlerId EventDispatcher::Subscribe(EventHandler handler) {
    std::lock_guard lock(_mutex);
    auto id = ++_nextId;
    _handlers.emplace_back(id, std::move(handler));
    return id;
}

void EventDispatcher::Unsubscribe(HandlerId id) {
    std::lock_guard lock(_mutex);
    auto it = std::ranges::find(_handlers, id, &Pair::first);
    if (it != _handlers.end()) {
        _handlers.erase(it);
    }
}

void EventDispatcher::Emit(const Event& event) {
    std::vector<EventHandler> handlersCopy;
    {
        std::lock_guard lock(_mutex);
        handlersCopy.reserve(_handlers.size());
        for (const auto& [_, handler] : _handlers) {
            handlersCopy.push_back(handler);
        }
    }
    // Call outside lock (avoids blocking all subscribers)
    for (auto& handler : handlersCopy) {
        handler(event);
    }
}