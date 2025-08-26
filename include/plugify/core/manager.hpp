#pragma once

#include "plugify/core/types.hpp"
#include "plugify/core/dependency_resolver.hpp"
#include "plugify/core/progress_reporter.hpp"
#include "plugify/core/file_system.hpp"
#include "plugify/core/manifest_parser.hpp"

#include <deque>
#include <ranges>

namespace plugify {
    // Stage Statistics
    /*struct StageStatistics {
        std::atomic<size_t> success;
        std::atomic<size_t> failed;
        Duration time;

        void AddSuccess(size_t op = 1) noexcept {
            success.fetch_add(op, std::memory_order_relaxed);
        }

        void AddFailed(size_t op = 1) noexcept {
            failed.fetch_add(op, std::memory_order_relaxed);
        }

        void SetTime(Duration t) noexcept {
            time = t;
        }

        size_t Total() const noexcept {
            return success.load(std::memory_order_relaxed) + failed.load(std::memory_order_relaxed);
        }

        float SuccessRate() const noexcept {
            auto total = Total();
            return total > 0 ? (100.0f * success.load()) / total : 0.0f;
        }

        std::string GetText(std::string_view name) const {
            return std::format("{:<12} ✓{:<6} ✗{:<6} {}ms ({:.1f}% success)",
                name, success.load(),failed.load(), time.count(), SuccessRate());
        }
    };*/



    // Initialization Statistics
    struct InitializationState {
        /*StageStatistics discovery{};
        /*StageStatistics parsing{};
        /*StageStatistics resolution{};
        /*StageStatistics load{};
        /*StageStatistics start{};
        /*StageStatistics update{};

        std::deque<std::pair<std::string, std::string>> errorLog;
        std::deque<std::pair<std::string, std::string>> warningLog;
        std::mutex logMutex;

        void AddError(std::string context, std::string message) {
            std::scoped_lock<std::mutex> lock(logMutex);
            errorLog.emplace_back(std::move(context), std::move(message));
        }
        void AddWarning(std::string context, std::string message) {
            std::scoped_lock<std::mutex> lock(logMutex);
            warningLog.emplace_back(std::move(context), std::move(message));
        }

        Duration TotalTime() const noexcept {
            return discovery.time + parsing.time + resolution.time + load.time + start.time;
        }

        std::string GetTextSummary() const {
            std::string buffer;
            auto it = std::back_inserter(buffer);

            std::format_to(it, "=== Initialization Summary ===\n");
            std::format_to(it, "{}\n", discovery.GetText("Discovery"));
            std::format_to(it, "{}\n", parsing.GetText("Parsing"));
            std::format_to(it, "{}\n", resolution.GetText("Resolution"));
            std::format_to(it, "{}\n", load.GetText("Loading"));
            std::format_to(it, "{}\n", start.GetText("Starting"));
            std::format_to(it, "Total Time: {}ms\n", TotalTime().count());

            if (!errorLog.empty()) {
                std::format_to(it, "\n=== Errors ({}) ===\n", errorLog.size());
                for (const auto& [ctx, msg] : errorLog | std::views::take(10)) {
                    std::format_to(it, "  [{}] {}\n", ctx, msg);
                }
            }

            if (!warningLog.empty()) {
                std::format_to(it, "\n=== Warnings ({}) ===\n", warningLog.size());
                for (const auto& [ctx, msg] : warningLog | std::views::take(10)) {
                    std::format_to(it, "  [{}] {}\n", ctx, msg);
                }
            }

            return buffer;
        }*/
    };

	class Plugify;
	class Manager {
	public:
		explicit Manager(Plugify& plugify);
		~Manager();

		// Dependency injection for customization
		//void setPackageDiscovery(std::unique_ptr<IPackageValidator> discovery);
		//void setPackageValidator(std::unique_ptr<IPackageValidator> validator);
		//void SetDependencyResolver(std::unique_ptr<IDependencyResolver> resolver);
		//void setModuleLoader(std::unique_ptr<IModuleLoader> loader);
		//void setPluginLoader(std::unique_ptr<IPluginLoader> loader);

		std::shared_ptr<InitializationState> Initialize();

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};

    class ManagerFactory {
    public:
        //static std::unique_ptr<IPackageDiscovery> createDefaultDiscovery();
        //static std::unique_ptr<IPackageValidator> createDefaultValidator();
        //static std::unique_ptr<IDependencyResolver> CreateDefaultResolver();
        //static std::unique_ptr<IModuleLoader> createDefaultModuleLoader();
        //static std::unique_ptr<IPluginLoader> createDefaultPluginLoader();
    };
}