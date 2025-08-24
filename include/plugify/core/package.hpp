#pragma once

#include <optional>

#include "plugify/core/error.hpp"
#include "plugify/core/manifest.hpp"

#include "date_time.hpp"

namespace plugify {
    // Package State Enum
    enum class PackageState {
        Initialized,
        Discovered,

        Parsing,
        Parsed,
        Corrupted,

        Resolving,
        Resolved,
        Unresolved,

        Disabled,
        Skipped,

        Loading,
        Loaded,
        Failed,

        Starting,
        Started,
        Stalled,

        Updating,
        Updated,
        /**/,

        Ending,
        Ended,

        Terminated
    };

    class Package;

    // Package Information with State
    struct alignas(std::hardware_destructive_interference_size) PackageInfo {
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;
        using Duration = std::chrono::milliseconds;

        struct Timings {
            std::array<Duration, static_cast<size_t>(PackageState::Terminated) + 1> _timepoints;

            Duration GetTotalTime() const {
                Duration total{0};
                for (const auto& t : _timepoints) {
                    total += t;
                }
                return total;
            }

            std::string ToString() const {
                std::string merged;
                merged.reserve(256);
                for (size_t i = 0; i < _timepoints.size(); ++i) {
                    auto& timing = _timepoints[i];
                    if (timing == Duration(0))
                        continue;
                    std::format_to(std::back_inserter(merged), "{} {}ms, ",
                        plg::enum_to_string(static_cast<PackageState>(i)), timing);
                }
                std::format_to(std::back_inserter(merged), "Total {}ms", GetTotalTime());
                return merged;
            }
        };

        UniqueId _id{};
        PackageType _type{PackageType::Unknown};
        PackageState _state{PackageState::Initialized};

        // Actual loaded instance (if any)
        std::shared_ptr<Package> _instance;
        std::shared_ptr<PackageManifest> _manifest;

        // Basic info
        //std::string _name;
        //std::string _language;
        std::filesystem::path _path;

        // Error tracking
        std::deque<std::string> _errors;
        std::deque<std::string> _warnings;

        // Timing information
        Timings _timings;
        TimePoint _lastOperationStart;

        void StartOperation(PackageState newState) {
            _lastOperationStart = Clock::now();
            SetState(newState);
        }

        void EndOperation(PackageState newState) {
            auto duration = std::chrono::duration_cast<Duration>(
                Clock::now() - _lastOperationStart
            );
            _timings._timepoints[static_cast<size_t>(newState)] = duration;
            SetState(newState);
        }

        UniqueId GetId() const { return _id; }
        PackageType GetType() const { return _type; }
        PackageState GetState() const { return _state; }
        const std::shared_ptr<Package>& GetInstance() const { return _instance; }
        const std::shared_ptr<PackageManifest>& GetManifest() const { return _manifest; }
        const std::filesystem::path& GetPath() const { return _path; }
        const std::deque<std::string>& GetErrors() const { return _errors; }
        const std::deque<std::string>& GetWarnings() const { return _warnings; }

        void SetId(UniqueId id) { _id = std::move(id); }
        void SetType(PackageType type) { _type = type; }
        void SetState(PackageState state) {
            //PL_ASSERT(IsValidTransition(_state, state) && "Invalid state transition");
            _state = state;
        }
        void SetManifest(std::shared_ptr<PackageManifest> manifest) {
            _manifest = std::move(manifest);
        }
        void SetInstance(std::shared_ptr<Package> instance) {
            _instance = std::move(instance);
        }
        void SetPath(std::filesystem::path path) {
            _path = std::move(path);
        }

        void AddError(std::string error) {
            _errors.emplace_back(std::move(error));
        }

        void AddWarning(std::string warning) {
            _warnings.emplace_back(std::move(warning));
        }

        void AddDependency(std::string_view dep) {
            if (_manifest) {
                if (!_manifest->dependencies) {
                    _manifest->dependencies = {};
                }
                _manifest->dependencies->emplace_back().SetName(dep);
            }
        }

        /*static bool IsValidTransition(PackageState from, PackageState to) {
            static const std::unordered_map<PackageState, std::unordered_set<PackageState>> transitions = {
                {PackageState::Initialized, {PackageState::Discovered}},
                {PackageState::Discovered, {PackageState::Parsed, PackageState::Corrupted}},
                {PackageState::Parsed, {PackageState::Resolved, PackageState::Unresolved}},
                {PackageState::Resolved, {PackageState::Loading, PackageState::Skipped, PackageState::Failed}},
                {PackageState::Loaded, {PackageState::Starting, PackageState::Started, PackageState::Failed}},
                {PackageState::Started, {PackageState::Updating, PackageState::Updated, PackageState::Failed}},
                {PackageState::Updated, {PackageState::Updating, PackageState::Updated, PackageState::Failed, PackageState::Ending}},
                {PackageState::Ending, {PackageState::Ended}},
            };

            auto it = transitions.find(from);
            return it != transitions.end() && it->second.contains(to);
        }*/

        static inline const std::string invalid = "<unknown>";
        const std::string& GetName() const { return _manifest ? _manifest->name : invalid; }
        const std::string& GetLanguage() const { return _manifest ? _manifest->language : invalid; }

        bool IsSlowOperation(Duration threshold = Duration(1000)) const {
            return _timings.GetTotalTime() > threshold;
        }

        std::string GetPerformanceReport() const {
            return std::format("{}: {}", GetName(), _timings.ToString());
        }
    };
}