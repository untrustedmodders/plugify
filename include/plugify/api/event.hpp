#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include <plugify/api/date_time.hpp>

namespace plugify {
	using PackageId = std::string;
	using ErrorMessage = std::string;

	enum class ErrorCode {
		None,
		FileNotFound,
		InvalidManifest,
		MissingDependency,
		VersionConflict,
		InitializationFailed,
		LanguageModuleNotLoaded,
		CircularDependency,
		ValidationFailed,
		DisabledByPolicy
	};

	struct Error {
		ErrorCode code{ErrorCode::None};
		ErrorMessage message;
		std::optional<std::filesystem::path> source;
	};

	enum class EventType {
		ModuleDiscovered,
		PluginDiscovered,
		ModuleLoading,
		ModuleLoaded,
		ModuleFailed,
		PluginLoading,
		PluginLoaded,
		PluginFailed,
		ConflictDetected,
		DependencyResolved
	};

	struct Event {
		EventType type{};
		DateTime ts{};
		PackageId packageId{};
		std::optional<Error> error{};
		std::unordered_map<std::string, std::string> data{};
	};

	using EventHandler = std::function<void(const Event&)>;
	using SubscriptionToken = std::uint64_t;
}