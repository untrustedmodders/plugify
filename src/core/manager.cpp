#include "plugify/assembly_loader.hpp"
#include "plugify/config.hpp"
#include "plugify/dependency_resolver.hpp"
#include "plugify/extension.hpp"
#include "plugify/manager.hpp"
#include "plugify/manifest.hpp"
#include "plugify/profiler.hpp"

#include "core/extension_loader.hpp"
#include "core/stages_impl.hpp"

using namespace plugify;

struct Manager::Impl {
	Impl(const ServiceLocator& srv, const Config& cfg) : services(srv), config(cfg) {
		// Create services
		assemblyLoader = services.Resolve<IAssemblyLoader>();
		fileSystem = services.Resolve<IFileSystem>();
		logger = services.Resolve<ILogger>();
		resolver = services.Resolve<IDependencyResolver>();
		profiler = services.TryResolve<IProfiler>();
	}

	~Impl() {
		if (initialized) {
			Terminate();
		}
	}

	const ServiceLocator& services;
	const Config& config;
	std::optional<Provider> provider;
	std::optional<ExtensionLoader> loader;
	std::vector<Extension> extensions;
	std::mutex lifecycleMutex;
	bool initialized{ false };

	// Services
	std::shared_ptr<IAssemblyLoader> assemblyLoader;
	std::shared_ptr<IFileSystem> fileSystem;
	std::shared_ptr<ILogger> logger;
	std::shared_ptr<IDependencyResolver> resolver;
	std::shared_ptr<IProfiler> profiler;

	// Dependency graphs (filled by resolution stage)
	std::vector<UniqueId> loadOrder;
	std::unordered_map<UniqueId, std::vector<UniqueId>> depGraph;
	std::unordered_map<UniqueId, std::vector<UniqueId>> reverseDepGraph;

	void Setup(const Manager& manager) {
		provider.emplace(services, config, manager);
		loader.emplace(services, config, *provider);
	}

public:
	Result<void> Initialize() {
		[[maybe_unused]] ScopedZone zone(profiler, PLUGIFY_SIGNATURE);

		std::lock_guard lock(lifecycleMutex);

		if (initialized) {
			return MakeError("Manager already initialized");
		}

		auto initialExtensions = DiscoverExtensions();
		if (!initialExtensions) {
			return MakeError(std::move(initialExtensions.error()));
		} /*else if (initialExtensions->empty()) {
			return MakeError("No extensions found");
		}*/
		extensions = std::move(*initialExtensions);

		if (extensions.empty()) {
			logger->Log(
				std::format(
					"No extensions found in '{}'. Each extension needs its own folder there, "
					"with a .pmodule or .pplugin manifest next to its binary.",
					plg::as_string(config.paths.extensionsDir)
				),
				Severity::Warning
			);
		}

		FailureTracker failureTracker(extensions.size());

		auto pipeline = Pipeline<Extension>::Create()
							.AddStage(std::make_unique<ParsingStage>(fileSystem))
							.AddStage(
								std::make_unique<ResolutionStage>(
									resolver,
									&loadOrder,
									&depGraph,
									&reverseDepGraph,
									config
								)
							)
							.AddStage(
								std::make_unique<LoadingStage>(
									*loader,
									failureTracker,
									depGraph,
									reverseDepGraph,
									config.loading.loadTimeout
								)
							)
							.AddStage(
								std::make_unique<ExportingStage>(
									*loader,
									failureTracker,
									depGraph,
									reverseDepGraph,
									config.loading.exportTimeout
								)
							)
							.AddStage(
								std::make_unique<StartingStage>(
									*loader,
									failureTracker,
									depGraph,
									reverseDepGraph,
									config.loading.startTimeout
								)
							)
							//.WithLogger(logger)
							//.WithProfiler(profiler)
							.WithThreadPoolSize(config.loading.maxConcurrentLoads)
							.Build();

		auto report = pipeline->Execute(extensions);

		// compute total failed items across stages
		size_t totalFailed = 0;
		for (const auto& [name, stats] : report.stages) {
			totalFailed += stats.failed;
		}

		// Print reports if configured
		if (config.logging.printReport) {
			logger->Log(report.Summary(), Severity::Info);
			logger->Log(loader->GetStatistics().Summary(), Severity::Info);
		}

		if (!extensions.empty()) {
			if (config.logging.printReport) {
				logger->Log("\n=== Extensions Report ===", Severity::Info);
				for (auto& ext : extensions) {
					logger->Log(ext.GetPerformanceReport(), Severity::Info);
				}
			}

			if (config.logging.printLoadOrder) {
				logger->Log(GenerateLoadOrder(), Severity::Info);
			}

			if (config.logging.printDependencyGraph) {
				logger->Log(GenerateDependencyGraph(), Severity::Info);
			}

			if (config.logging.printDigraphDot) {
				auto graph = GenerateDependencyGraphDOT();

				const auto& exportPath = config.logging.exportDigraphDot;
				if (exportPath.empty()) {
					// Nowhere to put it, so the log is the only way to get it out
					logger->Log(graph, Severity::Info);
				} else if (auto result = fileSystem->WriteTextFile(exportPath, graph); !result) {
					logger->Log(std::format("Export DOT: {}", result.error()), Severity::Error);
					logger->Log(graph, Severity::Info);
				} else {
					logger->Log(
						std::format("Export DOT: {}", plg::as_string(exportPath)),
						Severity::Info
					);
				}
			}
		}

		// Why anything is not running, in words that name the extension and the
		// missing piece. Worth saying even when the pipeline itself succeeded:
		// an extension silently dropped during resolution is the hardest kind of
		// failure to track down.
		auto problems = GenerateProblemReport();

		if (totalFailed > 0) {
			// The caller surfaces what we return, so the stage-by-stage breakdown
			// only goes to the log, where it is useful for debugging Plugify itself.
			logger->Log(report.Error(), Severity::Debug);
			return MakeError(problems.empty() ? report.Error() : problems);
		}

		if (!problems.empty()) {
			logger->Log(problems, Severity::Error);
		}

		initialized = true;
		return {};
	}

	void Update(std::chrono::milliseconds deltaTime) {
		[[maybe_unused]] ScopedZone zone(profiler, PLUGIFY_SIGNATURE);

		std::lock_guard lock(lifecycleMutex);

		if (!initialized) {
			return;
		}

		for (auto& ext : extensions) {
			if (ext.GetState() == ExtensionState::Running) {
				Result<void> result;
				switch (ext.GetType()) {
					case ExtensionType::Module: {
						result = loader->UpdateModule(ext, deltaTime);
						break;
					}

					case ExtensionType::Plugin: {
						result = loader->UpdatePlugin(ext, deltaTime);
						break;
					}

					default: {
						result = MakeError("Unknown extension type");
						break;
					}
				}
				if (!result) {
					logger->Log(result.error(), Severity::Error);
				}
			}
		}

		if (profiler) {
			profiler->MarkFrame("PlugifyLoop");
		}
	}

	void Terminate() {
		[[maybe_unused]] ScopedZone zone(profiler, PLUGIFY_SIGNATURE);

		std::lock_guard lock(lifecycleMutex);

		if (!initialized) {
			return;
		}

		for (auto it = extensions.rbegin(); it != extensions.rend(); ++it) {
			auto& ext = *it;
			if (ext.GetState() == ExtensionState::Running) {
				ext.StartOperation(ExtensionState::Ending);
				Result<void> result;
				switch (ext.GetType()) {
					case ExtensionType::Module: {
						// Modules typically don't need ending
						result = {};
						break;
					}

					case ExtensionType::Plugin: {
						result = loader->EndPlugin(ext);
						break;
					}

					default: {
						result = MakeError("Unknown extension type");
						break;
					}
				}
				if (!result) {
					logger->Log(result.error(), Severity::Error);
				}
				ext.EndOperation(ExtensionState::Ended);
			}
		}

		for (auto it = extensions.rbegin(); it != extensions.rend(); ++it) {
			auto& ext = *it;
			if (ext.GetState() == ExtensionState::Ended) {
				ext.StartOperation(ExtensionState::Terminating);
				Result<void> result;
				switch (ext.GetType()) {
					case ExtensionType::Module: {
						result = loader->UnloadModule(ext);
						break;
					}

					case ExtensionType::Plugin: {
						result = loader->UnloadPlugin(ext);
						break;
					}

					default: {
						result = MakeError("Unknown extension type");
						break;
					}
				}
				if (!result) {
					logger->Log(result.error(), Severity::Error);
				}
				ext.EndOperation(ExtensionState::Terminated);
			}
		}

		initialized = false;
	}

	Result<std::vector<Extension>> DiscoverExtensions() const {
		std::vector<std::filesystem::path> manifestPaths;
		std::vector<std::filesystem::path> directoriesToProcess;

		// Start with the root extensions directory
		directoriesToProcess.push_back(config.paths.extensionsDir);

		while (!directoriesToProcess.empty()) {
			auto currentDir = std::move(directoriesToProcess.back());
			directoriesToProcess.pop_back();

			// List contents of current directory
			auto entries = fileSystem->ListDirectory(currentDir);
			if (!entries) {
				// Keep going through the other directories, but say so: an
				// unreadable extensions directory otherwise looks exactly like an
				// empty one, and the user is left wondering where their plugin went.
				logger->Log(
					std::format(
						"Skipped '{}': {}",
						plg::as_string(currentDir),
						entries.error()
					),
					Severity::Warning
				);
				continue;
			}

			bool foundManifest = false;
			std::vector<std::filesystem::path> subdirs;

			// First pass: look for manifest files and collect subdirectories
			for (auto&& entry : *entries) {
				if (entry.is_regular_file()) {
					if (Extension::GetExtensionType(entry.path) != ExtensionType::Unknown) {
						manifestPaths.push_back(std::move(entry.path));
						foundManifest = true;
					}
				} else if (entry.is_directory()) {
					if (!config.security.excludedDirs.contains(entry.path.filename())) {
						subdirs.push_back(std::move(entry.path));
					}
				}
			}

			// Only recurse into subdirectories if no manifest was found
			if (!foundManifest) {
				directoriesToProcess.insert( //-V823
					directoriesToProcess.end(),
					std::make_move_iterator(subdirs.begin()),
					std::make_move_iterator(subdirs.end())
				);
			}
		}

		UniqueId id{ 0 };

		std::vector<Extension> exts;
		exts.reserve(manifestPaths.size());
		for (auto&& path : manifestPaths) {
			exts.emplace_back(id++, std::move(path));
		}
		return exts;
	}

	// What the user reads when something did not come up: every extension that is
	// not running, the errors it collected on the way, and where the missing
	// pieces belong. Empty when all is well.
	std::string GenerateProblemReport() const {
		std::vector<const Extension*> problems;
		size_t running = 0;
		size_t disabled = 0;

		for (const auto& ext : extensions) {
			switch (ext.GetState()) {
				case ExtensionState::Running:
					++running;
					break;
				case ExtensionState::Disabled:
					++disabled;
					[[fallthrough]];
				default:
					problems.push_back(&ext);
					break;
			}
		}

		// Extensions turned off on purpose are not a problem to report on their own.
		if (problems.size() == disabled) {
			return {};
		}

		// Which language modules are installed is the most useful piece of context
		// here: a plugin without its module is the most common way to get here.
		std::map<std::string, std::string> languages;
		for (const auto& ext : extensions) {
			if (ext.IsModule() && !ext.GetLanguage().empty()) {
				languages.emplace(ext.GetLanguage(), ext.GetName());
			}
		}

		std::string buffer;
		buffer.reserve(INITIAL_BUFFER_SIZE);

		auto it = std::back_inserter(buffer);

		std::format_to(
			it,
			"\n=== Plugify: {} of {} extension(s) did not start ===\n\n",
			problems.size(),
			extensions.size()
		);

		for (const Extension* ext : problems) {
			// A manifest that did not parse has neither a name nor a version yet,
			// so the path is all there is to go on.
			std::format_to(
				it,
				"  {} {} [{}, {}] - {}\n",
				ext->GetName().empty() ? "<unnamed>" : ext->GetName(),
				ext->GetVersionString().empty() ? "?" : ext->GetVersionString(),
				plg::enum_to_string(ext->GetType()),
				ext->GetLanguage().empty() ? "?" : ext->GetLanguage(),
				plg::enum_to_string(ext->GetState())
			);
			std::format_to(it, "    at '{}'\n", plg::as_string(ext->GetLocation()));

			// A plugin whose language has nothing to run it never gets a dependency
			// to fail on, so this is the only place the missing module is named -
			// unless it got as far as the loading stage, which says so itself.
			bool noModule = ext->IsPlugin() && !ext->GetLanguage().empty()
							&& !languages.contains(ext->GetLanguage())
							&& ext->GetState() != ExtensionState::Failed;
			if (noModule) {
				AppendBlock(
					buffer,
					"! ",
					std::format("No language module for '{}' is installed", ext->GetLanguage())
				);
			}

			if (!noModule && !ext->HasErrors() && !ext->HasWarnings()) {
				// Nothing to say about this one in particular: the pipeline stopped
				// at an earlier stage, before it was its turn.
				AppendBlock(buffer, "! ", "Never processed - startup stopped on an earlier failure");
			}

			for (const auto& error : ext->GetErrors()) {
				AppendBlock(buffer, "! ", error);
			}
			for (const auto& warning : ext->GetWarnings()) {
				AppendBlock(buffer, "~ ", warning);
			}

			std::format_to(it, "\n");
		}

		std::format_to(
			it,
			"Running: {}, not started: {} (of which disabled on purpose: {}).\n",
			running,
			problems.size(),
			disabled
		);

		if (languages.empty()) {
			std::format_to(it, "Installed language modules: none.\n");
		} else {
			std::format_to(it, "Installed language modules:");
			for (const auto& [language, module] : languages) {
				std::format_to(it, " {} ({})", language, module);
			}
			std::format_to(it, "\n");
		}
		std::format_to(
			it,
			"Extensions directory: '{}'\n"
			"Each extension needs its own folder there, with a .pmodule or .pplugin manifest\n"
			"next to its binary, together with everything it depends on.",
			plg::as_string(config.paths.extensionsDir)
		);

		return buffer;
	}

	// Writes a possibly multi-line message as one indented block, so the
	// "-> do this" hints stay attached to the error they belong to.
	static void AppendBlock(std::string& buffer, std::string_view marker, std::string_view message) {
		constexpr std::string_view indent = "    ";

		auto it = std::back_inserter(buffer);

		for (bool first = true; !message.empty(); first = false) {
			auto eol = message.find('\n');
			std::format_to(it, "{}{}{}\n", indent, first ? marker : "  ", message.substr(0, eol));
			if (eol == std::string_view::npos) {
				break;
			}
			message.remove_prefix(eol + 1);
		}
	}

#pragma region Reports
	static constexpr size_t INITIAL_BUFFER_SIZE = 4096;

	std::string_view NameOf(UniqueId id) const {
		auto it = std::ranges::find(extensions, id, &Extension::GetId);
		return it != extensions.end() ? it->GetName() : "<unknown>";
	}

	std::string GenerateLoadOrder() const {
		std::string buffer;
		buffer.reserve(INITIAL_BUFFER_SIZE);

		auto it = std::back_inserter(buffer);

		std::format_to(it, "\n=== Load Order ===\n");

		if (loadOrder.empty()) {
			std::format_to(it, "(empty)\n\n");
			return buffer;
		}

		for (size_t i = 0; i < loadOrder.size(); ++i) {
			const auto& id = loadOrder[i];
			std::format_to(it, "{:3}: {} (id={})\n", i, NameOf(id), id);
		}

		std::format_to(it, "\n");
		return buffer;
	}

	std::string GenerateDependencyGraph() const {
		std::string buffer;
		buffer.reserve(INITIAL_BUFFER_SIZE);

		auto it = std::back_inserter(buffer);

		std::format_to(it, "\n=== Dependency Graph (pkg -> [deps]) ===\n");

		if (depGraph.empty()) {
			std::format_to(it, "(empty)\n\n");
			return buffer;
		}

		for (const auto& [id, deps] : depGraph) {
			std::format_to(it, "{} (id={}) -> ", NameOf(id), id);
			if (deps.empty()) {
				std::format_to(it, "[]\n");
				continue;
			}

			std::format_to(it, "[");
			bool first = true;
			for (const auto& d : deps) {
				if (!first) {
					std::format_to(it, ", ");
				}
				std::format_to(it, "{} (id={})", NameOf(d), d);
				first = false;
			}
			std::format_to(it, "]\n");
		}

		std::format_to(it, "\n");
		return buffer;
	}

	std::string GenerateDependencyGraphDOT() const {
		std::string buffer;
		buffer.reserve(INITIAL_BUFFER_SIZE);

		auto it = std::back_inserter(buffer);

		std::format_to(it, "digraph packages {{\n");
		std::format_to(it, "  rankdir=LR;\n");

		// Nodes are named after the extension id, because that is what the edges
		// below are keyed by. Numbering them by their position in `extensions`
		// wires up the wrong nodes: ids are handed out in discovery order, while
		// the vector is rebuilt in load order.
		for (const auto& ext : extensions) {
			std::format_to(
				it,
				"  node{} [label=\"{}\\n(id={})\"];\n",
				ext.GetId(),
				ext.GetName(),
				ext.GetId()
			);
		}

		// edges
		for (const auto& [id, deps] : depGraph) {
			for (auto dep : deps) {
				std::format_to(it, "  node{} -> node{};\n", id, dep);
			}
		}

		std::format_to(it, "}}\n");
		return buffer;
	}
#pragma endregion Reports
};

Manager::Manager(const ServiceLocator& services, const Config& config)
	: _impl(std::make_unique<Impl>(services, config)) {
	_impl->Setup(*this);
}

Manager::~Manager() = default;

Result<void> Manager::Initialize() const {
	return _impl->Initialize();
}

bool Manager::IsInitialized() const {
	return _impl->initialized;
}

void Manager::Update(std::chrono::milliseconds deltaTime) const {
	return _impl->Update(deltaTime);
}

void Manager::Terminate() const {
	return _impl->Terminate();
}

// Query operations
bool Manager::IsExtensionLoaded(std::string_view name, std::optional<Constraint> constraint) const noexcept {
	auto it = std::ranges::find(_impl->extensions, name, &Extension::GetName);
	if (it != _impl->extensions.end()) {
		return !constraint || constraint->contains(it->GetVersion());
	}
	return false;
}

const Extension* Manager::FindExtension(std::string_view name) const noexcept {
	auto it = std::ranges::find(_impl->extensions, name, &Extension::GetName);
	if (it != _impl->extensions.end()) {
		return &(*it);
	}
	return nullptr;
}

const Extension* Manager::FindExtension(UniqueId id) const noexcept {
	auto it = std::ranges::find(_impl->extensions, id, &Extension::GetId);
	if (it != _impl->extensions.end()) {
		return &(*it);
	}
	return nullptr;
}

std::vector<const Extension*> Manager::GetExtensions() const {
	std::vector<const Extension*> result;
	for (auto& ext : _impl->extensions) {
		result.push_back(&ext);
	}
	return result;
}

std::vector<const Extension*> Manager::GetExtensionsByState(ExtensionState state) const {
	std::vector<const Extension*> result;
	for (auto& ext : _impl->extensions) {
		if (ext.GetState() == state) {
			result.push_back(&ext);
		}
	}
	return result;
}

std::vector<const Extension*> Manager::GetExtensionsByType(ExtensionType type) const {
	std::vector<const Extension*> result;
	for (auto& ext : _impl->extensions) {
		if (ext.GetType() == type) {
			result.push_back(&ext);
		}
	}
	return result;
}

std::string Manager::GenerateLoadOrder() const {
	return _impl->GenerateLoadOrder();
}

std::string Manager::GenerateDependencyGraph() const {
	return _impl->GenerateDependencyGraph();
}

std::string Manager::GenerateDependencyGraphDOT() const {
	return _impl->GenerateDependencyGraphDOT();
}

bool Manager::operator==(const Manager& other) const noexcept = default;
auto Manager::operator<=>(const Manager& other) const noexcept = default;
