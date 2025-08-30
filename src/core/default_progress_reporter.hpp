#pragma once

#include "plugify/core/logger.hpp"
#include "plugify/core/progress_reporter.hpp"

namespace plugify {
    class DefaultProgressReporter final : public IProgressReporter {
    public:
        DefaultProgressReporter(std::shared_ptr<ILogger> logger) : _logger(std::move(logger)) {}

        void OnStageStart(std::string_view stage, size_t totalItems) override {
            _logger->Log(std::format("[{}] Starting - {} items", stage, totalItems), Severity::Info);
        }

        void OnItemComplete(std::string_view stage, std::string_view itemId, bool success) override {
            _logger->Log( std::format("[{}] {} {}", stage, success ? 'v' : 'x', itemId), Severity::Info);
        }

        void OnStageComplete(std::string_view stage, size_t succeeded, size_t failed) override {
            _logger->Log(std::format("[{}] Complete - v {}, x {}", stage, succeeded, failed), Severity::Info);
        }

    private:
        std::shared_ptr<ILogger> _logger;
    };
}