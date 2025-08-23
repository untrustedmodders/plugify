#pragma once

#include <optional>

#include "plugify/core/error.hpp"
#include "plugify/core/manifest.hpp"

#include "date_time.hpp"

namespace plugify {
    // Package State Enum
    enum class PackageState {
        Initialized,
        ValidationFailed,
        ValidationPassed,
        DependencyResolved,
        Skipped,
        Loading,
        Loaded,
        Starting,
        Started,
        Failed,
        Terminated
    };

    // Package Information with State
    struct PackageInfo {
        std::shared_ptr<PackageManifest> manifest;
        //PackageId id;
        //Version version;
        //std::filesystem::path path;
        PackageState state{PackageState::Initialized};
        PackageType type{PackageType::Unknown};
        //std::vector<PackageId> dependencies;
        //std::vector<PackageId> dependents;
        //bool isLanguageModule = false;
        //int priority = 0;

        // Error tracking
        std::vector<std::string> errors;
        std::vector<std::string> warnings;

        // Timing information
        std::chrono::steady_clock::time_point initializedTime;
        std::chrono::steady_clock::time_point validationTime;
        std::chrono::steady_clock::time_point loadStartTime;
        std::chrono::steady_clock::time_point loadEndTime;
        std::chrono::steady_clock::time_point startStartTime;
        std::chrono::steady_clock::time_point startEndTime;

        // Package instance (once loaded)
        std::shared_ptr<void> instance;
    };

    //using Package = std::shared_ptr<PackageInfo>;
}