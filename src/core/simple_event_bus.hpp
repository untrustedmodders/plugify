#pragma once

#include "plugify/event_bus.hpp"

namespace plugify {
	// Simple event bus implementation
	class SimpleEventBus final : public IEventBus {
	public:
		SubscriptionId Subscribe(std::string_view eventType, EventHandler handler) override {
			std::unique_lock lock(_mutex);
			auto id = _nextId++;
			_handlers[std::string(eventType)].emplace_back(id, std::move(handler));
			return id;
		}

		void Unsubscribe(SubscriptionId id) override {
			std::unique_lock lock(_mutex);
			for (auto& [_, handlers] : _handlers) {
				handlers.erase(
					std::remove_if(
						handlers.begin(),
						handlers.end(),
						[id](const auto& pair) { return pair.first == id; }
					),
					handlers.end()
				);
			}
		}

		void Publish(std::string_view eventType, std::any data) override {
			std::vector<EventHandler> handlersToCall;

			{
				std::shared_lock lock(_mutex);
				auto it = _handlers.find(eventType);
				if (it != _handlers.end()) {
					for (const auto& [_, handler] : it->second) {
						handlersToCall.push_back(handler);
					}
				}
			}

			// Call handlers outside the lock to avoid deadlocks
			for (const auto& handler : handlersToCall) {
				handler(data);
			}
		}

	private:
		mutable std::shared_mutex _mutex;
		std::unordered_map<
			std::string,
			std::vector<std::pair<SubscriptionId, EventHandler>>,
			plg::case_insensitive_hash,
			plg::case_insensitive_equal
		> _handlers;
		SubscriptionId _nextId = 1;
	};
}
