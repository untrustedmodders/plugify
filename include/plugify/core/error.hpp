#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "plugify/core/types.hpp"

namespace plugify {
    enum class PackageState;

    // Error information structure
    struct PackageError {
        PackageId packageId;
        std::string stage; // "discovery", "parsing", "validation", "loading"
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        std::exception_ptr exception;
    };
}