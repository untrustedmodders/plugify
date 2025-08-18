#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include <plugify/api/date_time.hpp>
#include <plugify/core/error.hpp>

namespace plugify {
	enum class EventType {
		ModuleDiscovered,
		PluginDiscovered,

		ModuleLoading,
		ModuleLoaded,
		ModuleFailed,

		PluginLoading,
		PluginLoaded,
		PluginFailed,

		PluginStarted,
		PluginStopped,

		ConflictDetected,
		DependencyResolved
	};

	struct Event {
		EventType type;
		DateTime timestamp;
		PackageId packageId;
		std::optional<Error> error;
		std::unordered_map<std::string, std::string> data;
	};

	using EventHandler = std::function<void(const Event&)>;
	using SubscriptionToken = std::uint64_t;
}
