#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include "date_time.hpp"
#include "plugify/core/error.hpp"
#include "plugify/core/manifest.hpp"

namespace plugify {
	enum class EventType {
		ModuleDiscovered,
		PluginDiscovered,

		ModuleValidated,
		PluginValidated,

		ModuleLoading,
		ModuleLoaded,
		ModuleFailed,

		PluginLoading,
		PluginLoaded,
		PluginFailed,

		PluginStarted,
		//PluginUpdate,
		PluginEnded,

		//ModuleUpdate,
		ConflictDetected,
		DependencyResolved
	};

	struct Event {
		EventType type;
		std::chrono::system_clock::time_point timestamp;
		PackageId packageId;
		PackageType packageType;
		std::optional<Error> error;
		std::unordered_map<std::string, std::string> data;
	};

	using EventHandler = std::function<void(const Event&)>;
    using HandlerId = std::size_t;

    class EventDispatcher {
        using Pair = std::pair<HandlerId, EventHandler>;
    public:

        HandlerId Subscribe(EventHandler handler);
        void Unsubscribe(HandlerId id);

        void Emit(const Event& event);

    private:
        std::vector<Pair> _handlers;
        std::mutex _mutex;
        HandlerId _nextId{0};
    };
}
