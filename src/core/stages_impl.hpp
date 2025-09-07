#pragma once

#include "core/failure_tracker.hpp"
#include "core/pipeline.hpp"
#include "core/stages.hpp"

namespace plugify {
    // ============================================================================
    // Concrete Stage Implementations
    // ============================================================================

    // Parsing Stage - Transform type
    class ParsingStage : public ITransformStage<Extension> {
        std::shared_ptr<IManifestParser> _parser;
        std::shared_ptr<IFileSystem> _fileSystem;

    public:
        ParsingStage(std::shared_ptr<IManifestParser> parser, std::shared_ptr<IFileSystem> fileSystem)
            : _parser(std::move(parser))
            , _fileSystem(std::move(fileSystem)) {
        }

        std::string_view GetName() const override {
            return "Parsing";
        }

        bool ShouldProcess(const Extension& item) const override {
            return item.GetState() == ExtensionState::Discovered;
        }

        Result<void>
        ProcessItem(Extension& ext, [[maybe_unused]] const ExecutionContext<Extension>& ctx) override {
            ext.StartOperation(ExtensionState::Parsing);

            auto content = _fileSystem->ReadTextFile(ext.GetLocation());
            if (!content) {
                ext.AddError(content.error());
                ext.EndOperation(ExtensionState::Corrupted);
                return MakeError2(std::move(content.error()));
            }

            auto manifest = _parser->Parse(*content, ext.GetType());
            if (!manifest) {
                ext.AddError(manifest.error());
                ext.EndOperation(ExtensionState::Corrupted);
                return MakeError2(std::move(manifest.error()));
            }

            ext.SetManifest(std::move(*manifest));
            ext.EndOperation(ExtensionState::Parsed);
            return {};
        }
    };

    // Resolution Stage - Barrier type (reorders and filters)
    class ResolutionStage : public IBarrierStage<Extension> {
        std::shared_ptr<IDependencyResolver> _resolver;
        std::vector<UniqueId>* _loadOrder;
        plg::flat_map<UniqueId, std::vector<UniqueId>>* _depGraph;
        plg::flat_map<UniqueId, std::vector<UniqueId>>* _reverseDepGraph;
        const Config& _config;

    public:
        ResolutionStage(
            std::shared_ptr<IDependencyResolver> resolver,
            std::vector<UniqueId>* loadOrder,
            plg::flat_map<UniqueId, std::vector<UniqueId>>* depGraph,
            plg::flat_map<UniqueId, std::vector<UniqueId>>* reverseDepGraph,
            const Config& config
        )
            : _resolver(std::move(resolver))
            , _loadOrder(loadOrder)
            , _depGraph(depGraph)
            , _reverseDepGraph(reverseDepGraph)
            , _config(config) {
        }

        std::string_view GetName() const override {
            return "Resolution";
        }

        Result<void> ProcessAll(
            std::vector<Extension>& items,
            [[maybe_unused]] const ExecutionContext<Extension>& ctx
        ) override {
            auto [filtered, excluded] = FilterByPolicy(items);

            if (filtered.empty()) {
                return RestoreResult("No valid extensions to resolve", items, filtered, excluded);
            }

            AddLanguageDependencies(filtered);

            auto report = _resolver->Resolve(filtered);

            if (report.isLoadOrderValid) {
                return RestoreResult("No valid loading order", items, filtered, excluded);
            }

            if (report.loadOrder.empty()) {
                return RestoreResult("No valid extensions to load", items, filtered, excluded);
            }

            BuildResultInOrder(items, filtered, excluded, report);

            // Store dependency graphs
            *_depGraph = std::move(report.dependencyGraph);
            *_reverseDepGraph = std::move(report.reverseDependencyGraph);
            *_loadOrder = std::move(report.loadOrder);

            return {};
        }

    private:
        // Filter extensions based on whitelist/blacklist
        std::pair<std::vector<Extension>, std::vector<Extension>>
        FilterByPolicy(std::vector<Extension>& items) const {
            std::vector<Extension> filtered;
            std::vector<Extension> excluded;

            filtered.reserve(items.size());
            // excluded.reserve(items.size());

            for (auto&& ext : items) {
                bool include = true;

                // Check parsed state
                if (ext.GetState() != ExtensionState::Parsed) {
                    include = false;
                }

                // Check whitelist
                if (include && !_config.security.whitelistedExtensions.empty()
                    && !_config.security.whitelistedExtensions.contains(ext.GetName())) {
                    include = false;
                }

                // Check blacklist
                if (include && !_config.security.blacklistedExtensions.empty()
                    && _config.security.blacklistedExtensions.contains(ext.GetName())) {
                    include = false;
                }

                // Check platform
                if (include && !IsSupportsPlatform(ext.GetPlatforms())) {
                    include = false;
                }

                if (include) {
                    ext.SetState(ExtensionState::Resolving);
                    filtered.push_back(std::move(ext));
                } else {
                    if (ext.GetState() == ExtensionState::Parsed) {
                        ext.SetState(ExtensionState::Disabled);
                        ext.AddWarning("Excluded due to policy");
                    }
                    excluded.push_back(std::move(ext));
                }
            }

            items.clear();

            return { std::move(filtered), std::move(excluded) };
        }

        // Build language registry and add language dependencies
        static void AddLanguageDependencies(std::vector<Extension>& items) {
            plg::flat_map<std::string_view, UniqueId> languages;

            for (const auto& ext : items) {
                if (ext.GetType() == ExtensionType::Module) {
                    languages[ext.GetLanguage()] = ext.GetId();
                }
            }

            for (auto& ext : items) {
                if (ext.GetType() == ExtensionType::Plugin) {
                    auto it = languages.find(ext.GetLanguage());
                    if (it != languages.end()) {
                        ext.AddDependency(ToShortString(it->second));
                    } else {
                        ext.AddDependency(ext.GetLanguage());
                        ext.AddError(std::format("Language module '{}' is missing", ext.GetLanguage()));
                    }
                }
            }
        }

        static Result<void> RestoreResult(
            std::string_view error,
            std::vector<Extension>& items,
            std::vector<Extension>& filtered,
            std::vector<Extension>& excluded
        ) {
            items.insert(
                items.end(),
                std::make_move_iterator(filtered.begin()),
                std::make_move_iterator(filtered.end())
            );
            items.insert(
                items.end(),
                std::make_move_iterator(excluded.begin()),
                std::make_move_iterator(excluded.end())
            );
            return MakeError(
                "{}: {} filtered, {} excluded by resolver",
                error,
                filtered.size(),
                excluded.size()
            );
        }

         static void BuildResultInOrder(
            std::vector<Extension>& result,
            std::vector<Extension>& filtered,
            std::vector<Extension>& excluded,
            const ResolutionReport& report
        ) {
            std::unordered_map<UniqueId, Extension> idToExtension;
            idToExtension.reserve(filtered.size());

            for (auto&& ext : filtered) {
                idToExtension.emplace(ext.GetId(), std::move(ext));
            }

            // Add resolved extensions in load order
            for (const auto& id : report.loadOrder) {
                if (auto it = idToExtension.find(id); it != idToExtension.end()) {
                    auto& ext = it->second;
                    ext.SetState(ExtensionState::Resolved);

                    result.push_back(std::move(ext));
                    idToExtension.erase(it);
                }
            }

            // Add unresolved extensions at the end
            for (auto& [id, ext] : idToExtension) {
                ext.SetState(ExtensionState::Unresolved);

                // Add resolution errors if any
                if (auto it = report.issues.find(id); it != report.issues.end()) {
                    for (const auto& issue : it->second) {
                        if (issue.isBlocking) {
                            ext.AddError(issue.GetDetailedDescription());
                        } else {
                            ext.AddWarning(issue.GetDetailedDescription());
                        }
                    }
                }

                result.push_back(std::move(ext));
            }

            // Add excluded extensions at the very end
            result.insert(
                result.end(),
                std::make_move_iterator(excluded.begin()),
                std::make_move_iterator(excluded.end())
            );
        }

        static bool IsSupportsPlatform(std::span<const std::string> supportedPlatforms) {
            return supportedPlatforms.empty() || std::any_of(supportedPlatforms.begin(), supportedPlatforms.end(), [](const std::string& platform) {
                return platform.contains(PLUGIFY_PLATFORM);
            });
        }
    };

    // Initialize Stage - Transform type
    /*class InitializeStage : public ITransformStage<Extension> {
        ExtensionLoader& _loader;
    public:
        InitializeStage(ExtensionLoader& loader) : _loader(loader) {}

        std::string_view GetName() const override { return "Initialization"; }

        bool ShouldProcess(const Extension& item) const override {
            return item.GetState() == ExtensionState::Resolved;
        }

        Result<void> ProcessItem(
            Extension& ext,
            [[maybe_unused]] const ExecutionContext<Extension>& ctx) override {

            ext.StartOperation(ExtensionState::Initializating);

            Result<void> result;
            switch (ext.GetType()) {
                case ExtensionType::Module: {
                    auto assemblyResult = _loader.PreloadAssembly(ext.GetRuntime(),
    ext.GetDirectories()); break;
                }

                case ExtensionType::Plugin: {

                    break;
                }

                default:
                    result = MakeError2("Unknown extension type");
            }

            ext.EndOperation(ExtensionState::Initialized);
            return {};
        }
    };*/

    // Uses CRTP (Curiously Recurring Template Pattern) for compile-time polymorphism
    // Derived classes must implement: DoProcessItem() [non-virtual]
    template <typename Derived>
    class BaseFailurePropagatingStage : public ISequentialStage<Extension> {
    protected:
        ExtensionLoader& _loader;
        FailureTracker& _failureTracker;
        const plg::flat_map<UniqueId, std::vector<UniqueId>>& _depGraph;
        const plg::flat_map<UniqueId, std::vector<UniqueId>>& _reverseDepGraph;
        std::chrono::milliseconds _timeout;

    public:
        BaseFailurePropagatingStage(
            ExtensionLoader& loader,
            FailureTracker& failureTracker,
            const plg::flat_map<UniqueId, std::vector<UniqueId>>& depGraph,
            const plg::flat_map<UniqueId, std::vector<UniqueId>>& reverseDepGraph,
            std::chrono::milliseconds timeout
        )
            : _loader(loader)
            , _failureTracker(failureTracker)
            , _depGraph(depGraph)
            , _reverseDepGraph(reverseDepGraph)
            , _timeout(timeout) {
        }

        bool ContinueOnError() const override {
            return true;
        }

        Result<void> ProcessItem(
            Extension& ext,
            size_t position,
            size_t total,
            const ExecutionContext<Extension>& ctx
        ) override {
            // Common dependency failure check
            if (_failureTracker.HasAnyDependencyFailed(ext, _reverseDepGraph)) {
                return HandleDependencyFailure(ext);
            }

            // Delegate to derived class for actual processing
            return static_cast<Derived*>(this)->DoProcessItem(ext, position, total, ctx);
        }

        // Make timeout configurable per stage
        std::chrono::milliseconds GetTimeout() const {
            return _timeout;
        }

    protected:
        // Add hook points for derived classes
        // virtual void OnDependencyFailure(Extension& ext) {}
        // virtual void OnProcessingStart(Extension& ext) {}
        // virtual void OnProcessingComplete(Extension& ext, bool success) {}

        // Handle dependency failure uniformly
        Result<void> HandleDependencyFailure(Extension& ext) {
            std::string_view failedDep = _failureTracker.GetFailedDependencyName(ext, _reverseDepGraph);

            ext.SetState(ExtensionState::Skipped);
            ext.AddError(std::format("Skipped: dependency '{}' failed", failedDep));
            _failureTracker.MarkFailed(ext.GetId());

            return MakeError("Dependency '{}' failed", failedDep);
        }

        // Handle operation failure uniformly
        void
        HandleOperationFailure(Extension& ext, const Result<void>& result, ExtensionState failedState) {
            ext.AddError(result.error());
            ext.EndOperation(failedState);
            _failureTracker.MarkFailed(ext.GetId());
            PropagateFailureToDirectDependents(ext);
        }

        // Check and add timeout warning
        void CheckTimeout(Extension& ext, ExtensionState state) {
            if (auto operationTime = ext.GetOperationTime(state); operationTime > _timeout) {
                ext.AddWarning(
                    std::format("{} took {} to complete", plg::enum_to_string(state), operationTime)
                );
            }
        }

    private:
        void PropagateFailureToDirectDependents(const Extension& failedExt) const {
            if (auto it = _depGraph.find(failedExt.GetId()); it != _depGraph.end()) {
                for (const auto& dependentId : it->second) {
                    _failureTracker.MarkFailed(dependentId);
                }
            }
        }
    };

    // ============================================================================
    // Loading Stage
    // ============================================================================

    class LoadingStage : public BaseFailurePropagatingStage<LoadingStage> {
        plg::flat_map<std::string_view, Extension*> _loadedModules;

    public:
        using BaseFailurePropagatingStage::BaseFailurePropagatingStage;

        std::string_view GetName() const override {
            return "Loading";
        }

        bool ShouldProcess(const Extension& item) const override {
            return item.GetState() == ExtensionState::Resolved;
        }

        // Non-virtual method called by base class via CRTP
        Result<void> DoProcessItem(
            Extension& ext,
            [[maybe_unused]] size_t position,
            [[maybe_unused]] size_t total,
            [[maybe_unused]] const ExecutionContext<Extension>& ctx
        ) {
            ext.StartOperation(ExtensionState::Loading);

            Result<void> result;
            switch (ext.GetType()) {
                case ExtensionType::Module: {
                    result = _loader.LoadModule(ext);
                    if (result) {
                        _loadedModules[ext.GetLanguage()] = &ext;
                    }
                    break;
                }

                case ExtensionType::Plugin: {
                    auto it = _loadedModules.find(ext.GetLanguage());
                    if (it == _loadedModules.end()) {
                        result = MakeError("Language module '{}' not found", ext.GetLanguage());
                    } else {
                        result = _loader.LoadPlugin(*it->second, ext);
                    }
                    break;
                }

                default:
                    result = MakeError2("Unknown extension type");
            }

            if (!result) {
                HandleOperationFailure(ext, result, ExtensionState::Failed);
                return result;
            }

            ext.EndOperation(ExtensionState::Loaded);

            CheckTimeout(ext, ExtensionState::Loading);

            // Set module as running as it not has start and export
            if (ext.GetType() == ExtensionType::Module) {
                ext.StartOperation(ExtensionState::Running);
            }
            return {};
        }
    };

    // ============================================================================
    // Exporting Stage
    // ============================================================================

    class ExportingStage : public BaseFailurePropagatingStage<ExportingStage> {
        std::vector<const Extension*> _runningModules;

    public:
        using BaseFailurePropagatingStage::BaseFailurePropagatingStage;

        std::string_view GetName() const override {
            return "Exporting";
        }

        bool ShouldProcess(const Extension& item) const override {
            return item.GetState() == ExtensionState::Loaded
                   && item.GetType() == ExtensionType::Plugin;
        }

        void
        Setup(std::span<Extension> items, [[maybe_unused]] const ExecutionContext<Extension>& ctx) override {
            for (const auto& ext : items) {
                if (ext.GetType() == ExtensionType::Module
                    && ext.GetState() == ExtensionState::Running) {
                    _runningModules.push_back(&ext);
                }
            }
        }

        Result<void> DoProcessItem(
            Extension& ext,
            [[maybe_unused]] size_t position,
            [[maybe_unused]] size_t total,
            [[maybe_unused]] const ExecutionContext<Extension>& ctx
        ) {
            ext.StartOperation(ExtensionState::Exporting);

            for (const auto& module : _runningModules) {
                auto result = _loader.MethodExport(*module, ext);
                if (!result) {
                    HandleOperationFailure(ext, result, ExtensionState::Failed);
                    return result;
                }
            }

            ext.EndOperation(ExtensionState::Exported);
            CheckTimeout(ext, ExtensionState::Exporting);
            return {};
        }
    };

    // ============================================================================
    // Starting Stage
    // ============================================================================

    class StartingStage : public BaseFailurePropagatingStage<StartingStage> {
    public:
        using BaseFailurePropagatingStage::BaseFailurePropagatingStage;

        std::string_view GetName() const override {
            return "Starting";
        }

        bool ShouldProcess(const Extension& item) const override {
            return item.GetState() == ExtensionState::Exported
                   && item.GetType() == ExtensionType::Plugin;
        }

        Result<void> DoProcessItem(
            Extension& ext,
            [[maybe_unused]] size_t position,
            [[maybe_unused]] size_t total,
            [[maybe_unused]] const ExecutionContext<Extension>& ctx
        ) {
            ext.StartOperation(ExtensionState::Starting);

            auto result = _loader.StartPlugin(ext);
            if (!result) {
                HandleOperationFailure(ext, result, ExtensionState::Failed);
                return result;
            }

            ext.EndOperation(ExtensionState::Started);
            CheckTimeout(ext, ExtensionState::Starting);

            ext.StartOperation(ExtensionState::Running);
            return {};
        }
    };

}