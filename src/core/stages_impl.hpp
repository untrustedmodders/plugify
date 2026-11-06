#pragma once

#include "core/failure_tracker.hpp"
#include "core/pipeline.hpp"
#include "core/stages.hpp"
#include "core/glaze_metadata.hpp"

namespace plugify {
	// ============================================================================
	// Concrete Stage Implementations
	// ============================================================================

	// Parsing Stage - Transform type
	class ParsingStage : public ITransformStage<Extension> {
		std::shared_ptr<IFileSystem> _fileSystem;
		std::flat_map<ExtensionType, valijson::Schema> _schemas;

	public:
		ParsingStage(std::shared_ptr<IFileSystem> fileSystem)
			: _fileSystem(std::move(fileSystem)) {
		}

		std::string GetName() const override {
			return "Parsing";
		}

		bool ShouldProcess(const Extension& item) const override {
			return item.GetState() == ExtensionState::Discovered;
		}

		Result<void> Setup([[maybe_unused]] std::span<Extension> items) override {
			// The schemas are built into the library, so one of them failing to load is
			// a problem with the build and not with anything the user wrote. Reported
			// here it is said once; left to ProcessItem it came out as "no schema
			// registered" against every manifest in the directory.
			auto module = LoadSchema(schemas::module);
			if (!module) {
				return MakeError(std::move(module.error()));
			}

			auto plugin = LoadSchema(schemas::plugin);
			if (!plugin) {
				return MakeError(std::move(plugin.error()));
			}

			_schemas[ExtensionType::Module] = std::move(*module);
			_schemas[ExtensionType::Plugin] = std::move(*plugin);
			return {};
		}

		Result<void> ProcessItem(Extension& ext) override {
			ext.StartOperation(ExtensionState::Parsing);

			auto manifest = ParseManifest(ext.GetLocation(), ext.GetType());
			if (!manifest) {
				ext.AddError(manifest.error());
				ext.EndOperation(ExtensionState::Corrupted);
				return MakeError(std::move(manifest.error()));
			}

			ext.SetManifest(std::move(*manifest));
			ext.EndOperation(ExtensionState::Parsed);
			return {};
		}

	private:
		Result<Manifest> ParseManifest(const std::filesystem::path& file, ExtensionType type) {
			auto content = _fileSystem->ReadTextFile(file);
			if (!content) {
				return MakeError(std::move(content.error()));
			}

			auto it = _schemas.find(type);
			if (it == _schemas.end()) {
				return MakeError("No JSON schema registered for {} type", plg::enum_to_string(type));
			}

			auto manifest = ReadJson<Manifest>(*content, it->second);
			if (manifest) {
				// Resolve() first: it links every by-name prototype/enum reference to
				// its definition, which Validate() then relies on being present.
				if (auto result = manifest->Resolve(); !result) {
					return MakeError("Manifest resolution failed: {}", result.error());
				}
				if (auto result = manifest->Validate(); !result) {
					return MakeError("Manifest validation failed: {}", result.error());
				}
			}
			return manifest;
		}
	};

	// Resolution Stage - Barrier type (reorders and filters)
	class ResolutionStage : public IBarrierStage<Extension> {
		std::shared_ptr<IDependencyResolver> _resolver;
		std::vector<UniqueId>& _loadOrder;
		std::unordered_map<UniqueId, std::vector<UniqueId>>& _depGraph;
		std::unordered_map<UniqueId, std::vector<UniqueId>>& _reverseDepGraph;
		const Config& _config;

	public:
		ResolutionStage(
			std::shared_ptr<IDependencyResolver> resolver,
			std::vector<UniqueId>& loadOrder,
			std::unordered_map<UniqueId, std::vector<UniqueId>>& depGraph,
			std::unordered_map<UniqueId, std::vector<UniqueId>>& reverseDepGraph,
			const Config& config
		)
			: _resolver(std::move(resolver))
			, _loadOrder(loadOrder)
			, _depGraph(depGraph)
			, _reverseDepGraph(reverseDepGraph)
			, _config(config) {
		}

		std::string GetName() const override {
			return "Resolution";
		}

		Result<void> ProcessAll(std::vector<Extension>& items) override {
			size_t discovered = items.size();

			// Nothing to resolve is not the same as nothing resolvable: an empty
			// extensions directory is answered with a warning further up, and there is
			// no load order to be wrong about.
			if (discovered == 0) {
				return {};
			}

			auto [filtered, excluded] = FilterByPolicy(items);

			if (filtered.empty()) {
				Restore(items, filtered, excluded);
				return MakeError(
					"No extension passed the parsing and policy checks ({} discovered)",
					discovered
				);
			}

			size_t candidates = filtered.size();

			AddLanguageDependencies(filtered);

			auto report = _resolver->Resolve(filtered);

			// Runs whatever the outcome, so that every extension ends up in a
			// final state carrying the reason it is in it.
			BuildResultInOrder(items, filtered, excluded, report);

			if (!report.isLoadOrderValid) {
				return MakeError("Dependency resolution did not produce a usable load order");
			}

			if (report.loadOrder.empty()) {
				return MakeError(
					"No extension could be resolved: all {} candidate(s) have unmet "
					"dependencies or conflicts",
					candidates
				);
			}

			_depGraph = std::move(report.dependencyGraph);
			_reverseDepGraph = std::move(report.reverseDependencyGraph);
			_loadOrder = std::move(report.loadOrder);

			return {};
		}

	private:
		std::pair<std::vector<Extension>, std::vector<Extension>> FilterByPolicy(
			std::vector<Extension>& items
		) const {
			std::vector<Extension> filtered;
			std::vector<Extension> excluded;

			filtered.reserve(items.size());

			for (auto&& ext : items) {
				// Anything that did not parse already carries its own error
				if (ext.GetState() != ExtensionState::Parsed) {
					excluded.push_back(std::move(ext));
					continue;
				}

				if (auto reason = CheckPolicy(ext)) {
					ext.SetState(ExtensionState::Disabled);
					ext.AddWarning(std::move(*reason));
					excluded.push_back(std::move(ext));
					continue;
				}

				ext.SetState(ExtensionState::Resolving);
				filtered.push_back(std::move(ext));
			}

			items.clear();

			return { std::move(filtered), std::move(excluded) };
		}

		// Why this extension is not even a candidate, in words the user can act on,
		// or nothing when it passes.
		std::optional<std::string> CheckPolicy(const Extension& ext) const {
			const auto& security = _config.security;

			if (!security.whitelistedExtensions.empty()
				&& !security.whitelistedExtensions.contains(ext.GetName())) {
				return "Not started because a whitelist is configured and it is not on it";
			}

			if (!security.blacklistedExtensions.empty()
				&& security.blacklistedExtensions.contains(ext.GetName())) {
				return "Blacklisted in the configuration";
			}

			if (!IsSupportsPlatform(ext.GetPlatforms())) {
				return std::format(
					"Not supported on " PLUGIFY_PLATFORM " platform; the manifest lists {}",
					plg::join(ext.GetPlatforms(), ", ")
				);
			}

			return std::nullopt;
		}

		static void AddLanguageDependencies(std::vector<Extension>& items) {
			std::flat_map<std::string_view, std::string_view> languages;

			for (const auto& ext : items) {
				if (ext.GetType() == ExtensionType::Module && !ext.GetLanguage().empty()) {
					languages[ext.GetLanguage()] = ext.GetName();
				}
			}

			for (auto& ext : items) {
				if (ext.GetType() != ExtensionType::Plugin) {
					continue;
				}

				// With no module for the language there is nothing to depend on;
				// the plugin then fails in the loading stage, which says so, and
				// the problem report names the missing module either way.
				auto it = languages.find(ext.GetLanguage());
				if (it != languages.end()) {
					ext.AddDependency(std::string(it->second));
				}
			}
		}

		static void Restore(
			std::vector<Extension>& items,
			std::vector<Extension>& filtered,
			std::vector<Extension>& excluded
		) {
			items.insert( //-V823
				items.end(),
				std::make_move_iterator(filtered.begin()),
				std::make_move_iterator(filtered.end())
			);
			items.insert( //-V823
				items.end(),
				std::make_move_iterator(excluded.begin()),
				std::make_move_iterator(excluded.end())
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
				idToExtension[ext.GetId()] = std::move(ext);
			}

			// Add resolved extensions in load order
			for (const auto& id : report.loadOrder) {
				if (auto it = idToExtension.find(id); it != idToExtension.end()) {
					auto& ext = it->second;
					ext.SetState(ExtensionState::Resolved);

					// Resolved is not the same as nothing to say: an order cycle leaves
					// every member loadable but unordered, and that belongs on them.
					if (auto found = report.issues.find(id); found != report.issues.end()) {
						for (const auto& issue : found->second) {
							if (!issue.isBlocking) {
								ext.AddWarning(issue.GetDetailedDescription());
							}
						}
					}

					result.push_back(std::move(ext));
					idToExtension.erase(it);
				}
			}

			// Add unresolved extensions at the end
			for (auto& [id, ext] : idToExtension) {
				ext.SetState(ExtensionState::Unresolved);

				// What the solver has to say about it: it alone knows about version
				// constraints, conflicts and cycles.
				if (auto it = report.issues.find(id); it != report.issues.end()) {
					for (const auto& issue : it->second) {
						if (issue.isBlocking) {
							ext.AddError(issue.GetDetailedDescription());
						} else {
							ext.AddWarning(issue.GetDetailedDescription());
						}
					}
				}

				if (!ext.HasErrors()) {
					ext.AddError(
						"Could not be placed in the load order; a dependency or conflict of "
						"another extension rules it out"
					);
				}

				result.push_back(std::move(ext));
			}

			// Add excluded extensions at the very end
			result.insert( //-V823
				result.end(),
				std::make_move_iterator(excluded.begin()),
				std::make_move_iterator(excluded.end())
			);
		}

		static bool IsSupportsPlatform(std::span<const std::string> supportedPlatforms) {
			return supportedPlatforms.empty()
				   || std::any_of(
					   supportedPlatforms.begin(),
					   supportedPlatforms.end(),
					   [](const std::string& platform) { return platform == PLUGIFY_PLATFORM; }
				   );
		}
	};

	// Uses CRTP (Curiously Recurring Template Pattern) for compile-time polymorphism
	template <typename Derived>
	class BaseFailurePropagatingStage : public ISequentialStage<Extension> {
	protected:
		ExtensionLoader& _loader;
		FailureTracker& _failureTracker;
		const std::unordered_map<UniqueId, std::vector<UniqueId>>& _depGraph;
		const std::unordered_map<UniqueId, std::vector<UniqueId>>& _reverseDepGraph;
		std::chrono::milliseconds _timeout;
		std::span<Extension> _extensions;

	public:
		BaseFailurePropagatingStage(
			ExtensionLoader& loader,
			FailureTracker& failureTracker,
			const std::unordered_map<UniqueId, std::vector<UniqueId>>& depGraph,
			const std::unordered_map<UniqueId, std::vector<UniqueId>>& reverseDepGraph,
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

		Result<void> Setup(std::span<Extension> items) override {
			// Held for the rest of the stage, which is safe because the barrier that is
			// allowed to reallocate the container has already run
			_extensions = items;

			if constexpr (requires(Derived& d) { d.DoSetup(items); }) {
				static_cast<Derived*>(this)->DoSetup(items);
			}

			return {};
		}

		Result<void> ProcessItem(Extension& ext) override {
			if (auto failedDep = _failureTracker.GetFailedDependency(ext, _depGraph)) {
				return HandleDependencyFailure(ext, failedDep);
			}

			// Every stage keeps the same book: enter the busy state, do the work, and
			// then either record the failure or leave the item in the state the next
			// stage looks for. Written out once per stage, it was three chances to
			// forget the timeout or to name the wrong state.
			ext.StartOperation(Derived::kBusy);

			// Not optional: a stage without it does no work at all and still reports
			// every item as a success, walking it from kBusy to kDone with nothing in
			// between. Nothing else notices, so this is the only thing that does.
			static_assert(
				requires(Derived& d, Extension& e) { d.DoProcessItem(e); },
				"a failure-propagating stage has to implement "
				"Result<void> DoProcessItem(Extension&)"
			);

			if (auto result = static_cast<Derived*>(this)->DoProcessItem(ext); !result) {
				HandleOperationFailure(ext, result, ExtensionState::Failed);
				return result;
			}

			ext.EndOperation(Derived::kDone);
			CheckTimeout(ext, Derived::kBusy);

			// Whatever the stage has to do once the item is through, if anything
			if constexpr (requires(Derived& d) { d.DoFinish(ext); }) {
				static_cast<Derived*>(this)->DoFinish(ext);
			}

			return {};
		}

		Result<void> Teardown(std::span<Extension> items) override {
			if constexpr (requires(Derived& d) { d.DoTeardown(items); }) {
				return static_cast<Derived*>(this)->DoTeardown(items);
			}

			return {};
		}

	protected:
		Result<void> HandleDependencyFailure(Extension& ext, UniqueId id) {
			auto it = std::ranges::find(_extensions, id, &Extension::GetId);
			std::string_view failedDep = it != _extensions.end() ? it->GetName() : std::string_view{"<unknown>"};

			ext.SetState(ExtensionState::Skipped);
			ext.AddError(std::format("Skipped: dependency '{}' failed", failedDep));
			_failureTracker.MarkFailed(ext.GetId());

			return MakeError("Dependency '{}' failed", failedDep);
		}

		void HandleOperationFailure(Extension& ext, const Result<void>& result, ExtensionState failedState) {
			ext.AddError(result.error());
			ext.EndOperation(failedState);
			_failureTracker.MarkFailed(ext.GetId());
			PropagateFailureToDirectDependents(ext);
		}

		void CheckTimeout(Extension& ext, ExtensionState state) {
			if (auto operationTime = ext.GetOperationTime(state); operationTime > _timeout) {
				ext.AddWarning(
					std::format("{} took {} to complete", plg::enum_to_string(state), operationTime)
				);
			}
		}

	private:
		// The dependents of a failed extension live in the reverse graph; the forward
		// one holds what it depended on, which has nothing to answer for.
		void PropagateFailureToDirectDependents(const Extension& failedExt) const {
			if (auto it = _reverseDepGraph.find(failedExt.GetId()); it != _reverseDepGraph.end()) {
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
		// Pointers into the container, which nothing after the barrier reallocates
		std::flat_map<std::string_view, const Extension*> _loadedModules;

	public:
		using BaseFailurePropagatingStage::BaseFailurePropagatingStage;

		static constexpr ExtensionState kBusy = ExtensionState::Loading;
		static constexpr ExtensionState kDone = ExtensionState::Loaded;

		std::string GetName() const override {
			return "Loading";
		}

		bool ShouldProcess(const Extension& item) const override {
			return item.GetState() == ExtensionState::Resolved;
		}

		Result<void> DoProcessItem(Extension& ext) {
			switch (ext.GetType()) {
				case ExtensionType::Module: {
					auto result = _loader.LoadModule(ext);
					if (result) {
						_loadedModules[ext.GetLanguage()] = &ext;
					}
					return result;
				}

				case ExtensionType::Plugin: {
					auto it = _loadedModules.find(ext.GetLanguage());
					if (it == _loadedModules.end()) {
						return MakeError("Language module '{}' not found", ext.GetLanguage());
					}
					return _loader.LoadPlugin(*it->second, ext);
				}

				default:
					return MakeError("Unknown extension type");
			}
		}

		// A module has nothing to export and nothing to start, so it goes straight on
		void DoFinish(Extension& ext) {
			if (ext.GetType() == ExtensionType::Module) {
				ext.StartOperation(ExtensionState::Running);
			}
		}
	};

	// ============================================================================
	// Exporting Stage
	// ============================================================================

	class ExportingStage : public BaseFailurePropagatingStage<ExportingStage> {
		std::vector<const Extension*> _runningModules;

	public:
		using BaseFailurePropagatingStage::BaseFailurePropagatingStage;

		static constexpr ExtensionState kBusy = ExtensionState::Exporting;
		static constexpr ExtensionState kDone = ExtensionState::Exported;

		std::string GetName() const override {
			return "Exporting";
		}

		bool ShouldProcess(const Extension& item) const override {
			return item.GetState() == ExtensionState::Loaded
				   && item.GetType() == ExtensionType::Plugin;
		}

		void DoSetup(std::span<Extension> items) {
			for (const auto& ext : items) {
				if (ext.GetType() == ExtensionType::Module
					&& ext.GetState() == ExtensionState::Running) {
					_runningModules.push_back(&ext);
				}
			}
		}

		// Hands the plugin's methods to every module that is up, which is what
		// fills in the function pointers the plugin's own code calls through
		Result<void> DoProcessItem(Extension& ext) {
			for (const auto& module : _runningModules) {
				if (auto result = _loader.MethodExport(*module, ext); !result) {
					return result;
				}
			}

			return {};
		}
	};

	// ============================================================================
	// Starting Stage
	// ============================================================================

	class StartingStage : public BaseFailurePropagatingStage<StartingStage> {
	public:
		using BaseFailurePropagatingStage::BaseFailurePropagatingStage;

		static constexpr ExtensionState kBusy = ExtensionState::Starting;
		static constexpr ExtensionState kDone = ExtensionState::Started;

		std::string GetName() const override {
			return "Starting";
		}

		bool ShouldProcess(const Extension& item) const override {
			return item.GetState() == ExtensionState::Exported
				   && item.GetType() == ExtensionType::Plugin;
		}

		Result<void> DoProcessItem(Extension& ext) {
			return _loader.StartPlugin(ext);
		}

		// Starting is the last thing that happens to a plugin
		void DoFinish(Extension& ext) {
			ext.StartOperation(ExtensionState::Running);
		}
	};
}
