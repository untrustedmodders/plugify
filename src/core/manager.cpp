#include "plugify/manager.hpp"
#include "plugify/assembly_loader.hpp"
#include "plugify/config.hpp"
#include "plugify/dependency_resolver.hpp"
#include "plugify/extension.hpp"
#include "plugify/manifest.hpp"
#include "plugify/manifest_parser.hpp"
//#include "plugify/progress_reporter.hpp"

// #include "asm/defer.hpp"
#include "core/extension_loader.hpp"
#include "core/stages_impl.hpp"

using namespace plugify;

struct Manager::Impl {
    Impl(const ServiceLocator& services, const Config& cfg, const Manager& manager)
        : config{cfg}
        , provider(services, config, manager)
        , loader(services, config, provider)
    {
        // Create services
        assemblyLoader = services.Resolve<IAssemblyLoader>();
        fileSystem = services.Resolve<IFileSystem>();
        logger = services.Resolve<ILogger>();
        manifestParser = services.Resolve<IManifestParser>();
        resolver = services.Resolve<IDependencyResolver>();
        //progressReporter = services.Resolve<IProgressReporter>();
        //metricsCollector = services.Resolve<IMetricsCollector>();
    }
    ~Impl() {
        if (initialized) {
            Terminate();
        }
    }

    const Config& config;
    Provider provider;
    ExtensionLoader loader;
    std::vector<Extension> extensions;
    std::mutex lifecycleMutex;
    bool initialized{false};

    // Services
    std::shared_ptr<IAssemblyLoader> assemblyLoader;
    std::shared_ptr<IFileSystem> fileSystem;
    std::shared_ptr<ILogger> logger;
    std::shared_ptr<IManifestParser> manifestParser;
    std::shared_ptr<IDependencyResolver> resolver;
    //std::shared_ptr<IProgressReporter> progressReporter;
    //std::shared_ptr<IMetricsCollector> metricsCollector;

    // Dependency graphs (filled by resolution stage)
    std::vector<UniqueId> loadOrder;
    plg::flat_map<UniqueId, std::vector<UniqueId>> depGraph;
    plg::flat_map<UniqueId, std::vector<UniqueId>> reverseDepGraph;

public:
    Result<void> Initialize() {
        std::lock_guard lock(lifecycleMutex);

        if (initialized) {
            return MakeError2("Manager already initialized");
        }

        auto initialExtensions = DiscoverExtensions();
        if (!initialExtensions) {
            return MakeError2(std::move(initialExtensions.error()));
        } else if (initialExtensions->empty()) {
            // depending on policy: return ok or error. I'll return ok but log a warning:
            // return MakeError2("No extensions found");
        }

        FailureTracker failureTracker(initialExtensions->size());
        
        auto pipeline = Pipeline<Extension>::Create()
            .AddStage(std::make_unique<ParsingStage>(manifestParser, fileSystem))
            .AddStage(std::make_unique<ResolutionStage>(resolver, &loadOrder, &depGraph, &reverseDepGraph, config))
            .AddStage(std::make_unique<LoadingStage>(loader, failureTracker, depGraph, reverseDepGraph, config.loading.loadTimeout))
            .AddStage(std::make_unique<ExportingStage>(loader, failureTracker, depGraph, reverseDepGraph, config.loading.exportTimeout))
            .AddStage(std::make_unique<StartingStage>(loader, failureTracker, depGraph, reverseDepGraph, config.loading.startTimeout))
            //.WithConfig(config)
            //.WithReporter(progressReporter)
            //.WithMetrics(metricsCollector)
            .WithThreadPoolSize(config.loading.maxConcurrentLoads)
            .Build();

        auto [finalExtensions, report] = pipeline->Execute(std::move(*initialExtensions));

        // compute total failed items across stages
        size_t totalFailed = 0;
        for (auto const& [name, stats] : report.stages) {
            totalFailed += stats.failed;
        }

        extensions = std::move(finalExtensions);

        // Print reports if configured
        if (config.logging.printReport) {
            logger->Log(report.ToString(), Severity::Info);
            logger->Log(loader.GetStatistics().ToString(), Severity::Info);
        }

        if (!extensions.empty()) {
            if (config.logging.printReport) {
                logger->Log("=== Extensions Report ===", Severity::Info);
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
                logger->Log(graph, Severity::Info);

                const auto& exportPath = config.logging.exportDigraphDot;
                if (!exportPath.empty()) {
                    if (auto writeResult = fileSystem->WriteTextFile(exportPath, graph); !writeResult) {
                        logger->Log(std::format("Export DOT: {}", writeResult.error()), Severity::Error);
                    } else {
                        logger->Log(std::format("Export DOT: {}", plg::as_string(exportPath)), Severity::Info);
                    }
                }
            }
        }

        if (totalFailed > 0) {
            return MakeError2(report.ToString());
        }

        initialized = true;
        return Result<void>{};
    }

    void Update(Duration deltaTime) {
        std::lock_guard lock(lifecycleMutex);

        if (!initialized) {
            return;
        }

        for (auto& ext : extensions) {
            if (ext.GetState() == ExtensionState::Running) {
                Result<void> result;
                switch (ext.GetType()) {
                    case ExtensionType::Module: {
                        result = loader.UpdateModule(ext, deltaTime);
                        break;
                    }

                    case ExtensionType::Plugin: {
                        result = loader.UpdatePlugin(ext, deltaTime);
                        break;
                    }

                    default:
                        result = MakeError2("Unknown extension type");
                }
                if (!result) {
                    logger->Log(result.error(), Severity::Error);
                }
            }
        }
    }

    void Terminate() {
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
                        result = loader.EndPlugin(ext);
                        break;
                    }

                    default:
                        result = MakeError2("Unknown extension type");
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
                        result = loader.UnloadModule(ext);
                        break;
                    }

                    case ExtensionType::Plugin: {
                        result = loader.UnloadPlugin(ext);
                        break;
                    }

                    default:
                        result = MakeError2("Unknown extension type");
                }
                if (!result) {
                    logger->Log(result.error(), Severity::Error);
                }
                ext.EndOperation(ExtensionState::Terminated);
            }
        }

        initialized = false;
    }

    const std::array<std::string_view, 2> MANIFEST_EXTENSIONS = {
        "*.pplugin", "*.pmodule"
    };

    Result<std::vector<Extension>> DiscoverExtensions() const {
        auto dirs = fileSystem->IterateDirectory(config.paths.extensionsDir, {
            .filter_predicate = [](const FileInfo& f) { return f.is_directory(); },
        });
        if (!dirs) {
            return MakeError2(std::move(dirs.error()));
        }

        std::vector<std::filesystem::path> result;
        for (const auto& dir : *dirs) {
            auto paths = fileSystem->FindFiles(dir.path, MANIFEST_EXTENSIONS, false);
            if (!paths) {
                return MakeError2(std::move(paths.error()));
            }

            if (paths->size() == 1) {
                result.emplace_back(std::move(paths->front()));
            }
        }

        if (result.empty()) {
            return MakeError2("Did not find any extensions in the directory");
        }

        UniqueId id{0};

        std::vector<Extension> exts;
        exts.reserve(result.size());
        for (auto&& path : result) {
            exts.emplace_back(id++, std::move(path));
        }
        return exts;
    }

    const size_t INITIAL_BUFFER_SIZE = 1024;
    const size_t REPORT_BUFFER_SIZE = 2048;
    const size_t DOT_BUFFER_SIZE = 4096;

    std::string GenerateLoadOrder() const {
        std::string buffer;
        buffer.reserve(INITIAL_BUFFER_SIZE); // preallocate some space to reduce reallocs

        auto it = std::back_inserter(buffer);

        std::format_to(it, "\n=== Load Order ===\n");

        if (loadOrder.empty()) {
            std::format_to(it, "(empty)\n\n");
            return buffer;
        }

        for (size_t i = 0; i < loadOrder.size(); ++i) {
            const auto& id = loadOrder[i];
            std::format_to(it, "{:3}: {} (id={})\n", i, ToShortString(id), id);
        }

        buffer.push_back('\n');
        return buffer;
    }

    std::string GenerateDependencyGraph() const {
        std::string buffer;
        buffer.reserve(REPORT_BUFFER_SIZE);

        auto it = std::back_inserter(buffer);

        std::format_to(it, "\n=== Dependency Graph (pkg -> [deps]) ===\n");

        if (depGraph.empty()) {
            std::format_to(it, "(empty)\n\n");
            return buffer;
        }

        for (const auto& [id, deps] : depGraph) {
            std::format_to(it, "{} (id={}) -> ", ToShortString(id), id);
            if (deps.empty()) {
                std::format_to(it, "[]\n");
                continue;
            }

            buffer.push_back('[');
            bool first = true;
            for (const auto& d : deps) {
                if (!first) {
                    std::format_to(it, ", ");
                }
                std::format_to(it, "{} (id={})", ToShortString(d), d);
                first = false;
            }
            std::format_to(it, "]\n");
        }

        buffer.push_back('\n');
        return buffer;
    }

    std::string GenerateDependencyGraphDOT() const {
        std::string buffer;
        buffer.reserve(DOT_BUFFER_SIZE);

        auto it = std::back_inserter(buffer);

        std::format_to(it, "digraph packages {{\n");
        std::format_to(it, "  rankdir=LR;\n");

        // nodes
        for (size_t i = 0; i < extensions.size(); ++i) {
            const auto& ext = extensions[i];
            std::format_to(it,
                           "  node{} [label=\"{}\\n(id={})\"];\n",
                           i, ext.GetName(), ext.GetId());
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
};

Manager::Manager(const ServiceLocator& services, const Config& config)
    : _impl(std::make_unique<Impl>(services, config, *this)) {}

Manager::~Manager() = default;

Result<void> Manager::Initialize() const {
    return _impl->Initialize();
}

bool Manager::IsInitialized() const {
    return _impl->initialized;
}

void Manager::Update(Duration deltaTime) const {
    return _impl->Update(deltaTime);
}

void Manager::Terminate() const {
    return _impl->Terminate();
}

// Query operations
bool Manager::IsExtensionLoaded(std::string_view name, std::optional<Constraint> constraint) const noexcept {
    auto it = std::find_if(_impl->extensions.begin(), _impl->extensions.end(),
        [&](const Extension& e) { return e.GetName() == name; });
    if (it != _impl->extensions.end()) {
        return !constraint || constraint->contains(it->GetVersion());
    }
    return false;
}

const Extension* Manager::FindExtension(std::string_view name) const noexcept {
    auto it = std::find_if(_impl->extensions.begin(), _impl->extensions.end(),
        [&](const Extension& e) { return e.GetName() == name; });
    if (it != _impl->extensions.end()) {
        return &(*it);
    }
    return nullptr;
}

const Extension* Manager::FindExtension(UniqueId id) const noexcept {
    auto it = std::find_if(_impl->extensions.begin(), _impl->extensions.end(),
        [&](const Extension& e) { return e.GetId() == id; });
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