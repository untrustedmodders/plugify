#pragma once

#include <optional>

#include "date_time.hpp"
#include "plugify/core/error.hpp"
#include "plugify/core/manifest.hpp"

namespace plugify {
	enum class PackageState {
		Discovered,
		Validated,
		Initializing,
		Ready,
		Started,
		Ended,
		Error,
		Disabled
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
	//using ModuleInfo = std::shared_ptr<Package<ModuleManifest>>;
}