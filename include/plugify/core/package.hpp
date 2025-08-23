#pragma once

#include <optional>

#include "date_time.hpp"
#include "plugify/core/error.hpp"
#include "plugify/core/manifest.hpp"

namespace plugify {
	enum class PackageState {
		Uninitialized,   // Not yet processed
		Discovered,      // Found on disk
		Validated,       // Manifest validated successfully
		Skipped,         // Skipped due to dependency failures or policy
		Initializing,    // Currently being initialized
		Ready,           // Successfully initialized
		Started,         // Successfully running
		Ended,           // Terminated/unloaded
		Error,           // Failed to initialize
		Disabled         // Disabled by configuration
	};

	template<typename Type>
	struct Package {
	    PackageId id;
		std::shared_ptr<Type> manifest;
		PackageState state{PackageState::Uninitialized};
		std::optional<Error> lastError;
		std::chrono::milliseconds discoveredAt;

	    // Helper methods
	    bool IsModule() const { return manifest->type == PackageType::Module; }
	    bool IsPlugin() const { return manifest->type == PackageType::Plugin; }
	    std::string_view GetTypeName() const { return IsModule() ? "module" : "plugin"; }
	    const std::string& GetLanguageName() const { return IsModule() ? AsModuleManifest()->language : AsPluginManifest()->language; }
	    std::shared_ptr<ModuleManifest> AsModuleManifest() const { return std::static_pointer_cast<ModuleManifest>(manifest); }
	    std::shared_ptr<PluginManifest> AsPluginManifest() const { return std::static_pointer_cast<PluginManifest>(manifest); }
	};

    using PackageInfo = std::shared_ptr<Package<PackageManifest>>;
    using PluginInfo = std::shared_ptr<Package<PluginManifest>>;
    using ModuleInfo = std::shared_ptr<Package<ModuleManifest>>;

}