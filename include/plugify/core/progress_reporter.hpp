#include once

#include <cstdint>
#include <string_view>

namespace plugify {

    // Progress Reporter Interface
    class IProgressReporter {
    public:
        virtual ~IProgressReporter() = default;

        virtual void OnPhaseStart(std::string_view phase, size_t totalItems) = 0;
        virtual void OnItemComplete(std::string_view phase, std::string_view itemId, bool success) = 0;
        virtual void OnPhaseComplete(std::string_view phase, size_t succeeded, size_t failed) = 0;
    };
}