#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>

#include "plugify/core/report.hpp"

namespace plugify {
    class PackageInitializer {
    public:
        struct InitializationContext {
            PackageType type;
            std::string typeName;  // "Module" or "Plugin"
            std::unordered_set<PackageId> successfullyLoaded;
            std::vector<std::shared_ptr<PackageInfo>> packagesToInit;
        };

        // Core initialization logic (shared between modules and plugins)
        virtual InitializationReport InitializePackages(InitializationContext& context);
    };
}