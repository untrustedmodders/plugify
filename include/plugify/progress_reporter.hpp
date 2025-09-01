#pragma once

#include <cstdint>
#include <string_view>

namespace plugify {
    // Progress Reporter Interface
    /*class IProgressReporter {
    public:
        virtual ~IProgressReporter() = default;

        virtual void OnStageStart(std::string_view stage, size_t totalItems) = 0;
        virtual void OnItemComplete(std::string_view stage, std::string_view itemId, bool success) = 0;
        virtual void OnStageComplete(std::string_view stage, size_t succeeded, size_t failed) = 0;
    };*/
}