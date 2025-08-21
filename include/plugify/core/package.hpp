#pragma once

#include <optional>

#include "date_time.hpp"
#include "plugify/core/error.hpp"
#include "plugify/core/manifest.hpp"

namespace plugify {
	enum class PackageState {
		Discovered,      // Found on disk
		Validated,       // Manifest validated successfully
		Skipped,         // Skipped due to dependency failures or policy  ← NEW
		Initializing,    // Currently being initialized
		Ready,           // Successfully initialized (modules)
		Started,         // Successfully initialized and running (plugins)
		Ended,           // Terminated/unloaded
		Error,           // Failed to initialize
		Disabled         // Disabled by configuration
	};

	template<typename Type>
	struct Package {
		std::shared_ptr<Type> manifest;
		PackageState state{PackageState::Discovered};
		std::optional<Error> lastError;
		DateTime discoveredAt;
	};

	using PackageInfo = std::shared_ptr<Package<PackageManifest>>;
	using PluginInfo = std::shared_ptr<Package<PluginManifest>>;
	using ModuleInfo = std::shared_ptr<Package<ModuleManifest>>;
}