#include "plugify/assembly_loader.hpp"
#include "plugify/plugify.hpp"

#include "core/console_logger.hpp"
#include "core/glaze_config_provider.hpp"
#include "core/glaze_manifest_parser.hpp"
#include "core/glaze_metadata.hpp"
#include "core/libsolv_dependency_resolver.hpp"
#include "core/simple_event_bus.hpp"
#include "core/standart_file_system.hpp"

#include "basic_assembly_loader.hpp"
#include "dummy_lifecycle.hpp"

using namespace plugify;

// Plugify::Impl
struct Plugify::Impl {
    ServiceLocator services;
    Config config;
    Manager manager;
    Version version;

    std::shared_ptr<ILogger> logger;
    std::shared_ptr<IEventBus> eventBus;
    std::shared_ptr<IFileSystem> fileSystem;

    std::thread::id ownerThreadId;
    std::jthread updateThread;
    bool initialized{false};
    bool shouldStop{false};

    static inline const std::thread::id mainThreadId = std::this_thread::get_id();

    Impl(ServiceLocator srv, Config cfg)
        : services(std::move(srv))
        , config(std::move(cfg))
        , manager(services, config)
        , ownerThreadId(std::this_thread::get_id()) {
        // Get version
        plg::parse(PLUGIFY_VERSION, version);
        // Create services
        logger = services.Resolve<ILogger>();
        eventBus = services.Resolve<IEventBus>();
        fileSystem = services.Resolve<IFileSystem>();

        // Set default logging level
        logger->SetLogLevel(config.logging.severity);
    }

    ~Impl() {
        if (initialized) {
            Terminate();
        }
    }

    Result<void> Initialize() {
        if (std::this_thread::get_id() != ownerThreadId) {
            throw std::runtime_error("Initialization called from non-owner thread");
        }

        if (initialized) {
            return std::unexpected("Already initialized");
        }

        // Initialize logging
        logger->Log("Initializing Plugify...", Severity::Info);

        // Load configuration
        /*if (auto configProvider = context->GetConfigProvider()) {
            if (auto fileSystem = context->GetFileSystem()) {
                auto configPath = paths.baseDir / paths.configsDir / "plugify.json";
                if (fileSystem->IsExists(configPath)) {
                    if (auto result = configProvider->LoadFromFile(configPath); !result) {
                        LogError(std::format("Failed to load config: {}", result.error()));
                        return std::unexpected("Failed to load config");
                    }
                }
            }
        }*/

        // Create necessary directories
        if (!CreateDirectories()) {
            return std::unexpected("Failed to create directories");
        }

        // Initialize manager
        //manager.Initialize();

        // Start update thread if configured
        StartUpdateThread();

        logger->Log("Plugify initialized successfully", Severity::Info);
        logger->Log(std::format("Version: {}", version), Severity::Info);
        logger->Log(std::format("Git: [{}]:({}) - {} on {} at '{}'", PLUGIFY_GIT_COMMIT_HASH, PLUGIFY_GIT_TAG, PLUGIFY_GIT_COMMIT_SUBJECT, PLUGIFY_GIT_BRANCH, PLUGIFY_GIT_COMMIT_DATE), Severity::Info);
        logger->Log(std::format("Compiled on: {} from: {} with: '{}'", PLUGIFY_COMPILED_SYSTEM, PLUGIFY_COMPILED_GENERATOR, PLUGIFY_COMPILED_COMPILER), Severity::Info);

        initialized = true;

        // Publish initialization event
        eventBus->Publish("plugify.initialized", std::any{});

        return {};
    }

    void Terminate() {
        if (std::this_thread::get_id() != ownerThreadId) {
            throw std::runtime_error("Termanation called from non-owner thread");
        }

        if (!initialized) {
            return;
        }

        logger->Log("Terminating Plugify...", Severity::Info);;

        // Stop update thread
        StopUpdateThread();

        // Terminate manager
        manager.Terminate();

        // Save configuration
        /*if (auto configProvider = context->GetConfigProvider()) {
            if (configProvider->IsDirty()) {
                auto configPath = paths.baseDir / paths.configsDir / "plugify.json";
                if (auto result = configProvider->SaveToFile(configPath); !result) {
                    LogError(std::format("Failed to save config: {}", result.error()));
                }
            }
        }*/
        
        // Publish termination event
        eventBus->Publish("plugify.terminating", std::any{});
        
        // Flush logs
        logger->Log("Plugify terminated", Severity::Info);
        logger->Flush();

        initialized = false;
    }

    void Update(std::chrono::microseconds deltaTime) {
        if (std::this_thread::get_id() != ownerThreadId) {
            throw std::runtime_error("Update called from non-owner thread");
        }

        if (!initialized) {
            return;
        }

        manager.Update(deltaTime);

        // Process events if event bus is available
        /*if (auto eventBus = context->GetEventBus()) {
            // Event bus would process pending events here
        }*/
    }

private:
    bool CreateDirectories() {
        if (!CheckDirectories()) {
            return false;
        }

        auto createDir = [&](const std::filesystem::path& dir) {
            if (!dir.empty() && !fileSystem->IsExists(dir)) {
                if (auto result = fileSystem->CreateDirectories(dir); !result) {
                    logger->Log(result.error(), Severity::Error);;
                } else {
                    logger->Log(std::format("Created directory: {}", plg::as_string(dir)), Severity::Info);
                }
            }
        };

        //createDir(config.paths.baseDir);
        createDir(config.paths.baseDir / config.paths.extensionsDir);
        createDir(config.paths.baseDir / config.paths.configsDir);
        createDir(config.paths.baseDir / config.paths.dataDir);
        createDir(config.paths.baseDir / config.paths.logsDir);
        createDir(config.paths.baseDir / config.paths.cacheDir);
        return true;
    }

    bool CheckDirectories() {
        struct PathValidation {
            std::string_view name;
            std::filesystem::path path;
            bool requireExists;   // Optional: check if directory should exist
            bool requireRelative; // Optional: check if path should be relative
            bool canCreate;       // Optional: whether we can create it if missing
        };

        const std::array<PathValidation, 6> pathsToValidate = {
            PathValidation{"baseDir", config.paths.baseDir, true, false, false},
            PathValidation{"extensionsDir", config.paths.extensionsDir, false, false, true},
            PathValidation{"configsDir", config.paths.configsDir, false, false, true},
            PathValidation{"dataDir", config.paths.dataDir, false, false, true},
            PathValidation{"logsDir", config.paths.logsDir, false, false, true},
            PathValidation{"cacheDir", config.paths.cacheDir, false, false, true}
        };

        // Validation result accumulator
        struct ValidationResult {
            std::vector<std::string> errors;
            std::vector<std::string> warnings;

            void AddError(std::string msg) {
                errors.push_back(std::move(msg));
            }

            void AddWarning(std::string msg) {
                warnings.push_back(std::move(msg));
            }

            bool IsSuccess() const {
                return errors.empty();
            }
        } result;

        // Step 1: Validate path format and existence
        for (const auto& [name, path, requireExists, requireRelative, canCreate] : pathsToValidate) {
            // Check if path is empty
            if (path.empty()) {
                result.AddError(std::format("{} is not configured", name));
                continue;
            }

            // Check if path is normalized
            if (path.lexically_normal() != path) {
                result.AddError(std::format("{} is not normalized: '{}'", name, plg::as_string(path)));
                continue;
            }

            // Check if path is relative (optional, depending on requirements)
            if (requireRelative && path.is_absolute()) {
                result.AddError(std::format("{} must be relative: '{}'", name, plg::as_string(path)));
                continue;
            }

            // Check existence if required
            bool exists = fileSystem->IsExists(path);

            if (requireExists && !exists) {
                result.AddError(std::format("{} does not exist: '{}'", name, plg::as_string(path)));
            } else if (!exists && canCreate) {
                result.AddWarning(std::format("{} will be created: '{}'", name, plg::as_string(path)));
            }
        }

        // Step 2: Check for path collisions
        auto checkCollisions = [&result](std::span<const PathValidation> paths, size_t startIdx = 0) {
            for (size_t i = startIdx; i < paths.size(); ++i) {
                if (paths[i].path.empty()) continue;

                for (size_t j = i + 1; j < paths.size(); ++j) {
                    if (paths[j].path.empty()) continue;

                    const auto& path1 = paths[i].path;
                    const auto& path2 = paths[j].path;

                    // Check if one path is a parent of another
                    auto [it1, it2] = std::mismatch(path1.begin(), path1.end(),
                                                   path2.begin(), path2.end());

                    if (it1 == path1.end() || it2 == path2.end()) {
                        result.AddError(
                            std::format("{} ('{}') conflicts with {} ('{}')",
                                       paths[i].name, plg::as_string(path1),
                                       paths[j].name, plg::as_string(path2))
                        );
                    }
                }
            }
        };

        // Check collisions (skip baseDir at index 0)
        checkCollisions(pathsToValidate, 1);

        // Log results
        if (!result.warnings.empty()) {
            logger->Log(std::format("Warnings: {}", plg::join(result.warnings, "; ")), Severity::Warning);
        }

        if (!result.errors.empty()) {
            logger->Log(std::format("Validation failed: {}", plg::join(result.errors, "; ")), Severity::Error);
        }

        return result.IsSuccess();
    }

    void StartUpdateThread() {
        if (config.runtime.updateInterval > Duration{0}) {
            updateThread = std::jthread([this](std::stop_token token) {
                auto lastTime = Clock::now();

                while (!token.stop_requested()) {
                    auto now = Clock::now();
                    auto deltaTime = std::chrono::duration_cast<Duration>(now - lastTime);
                    lastTime = now;

                    manager.Update(deltaTime);

                    std::this_thread::sleep_for(config.runtime.updateInterval);
                }
            });
        }
    }

    void StopUpdateThread() {
        if (updateThread.joinable()) {
            updateThread.request_stop();
            updateThread.join();
        }
    }
};

// Plugify implementation
Plugify::Plugify(ServiceLocator services, Config config)
    : _impl(std::make_unique<Impl>(std::move(services), std::move(config))) {
}

Plugify::~Plugify() = default;

Result<void> Plugify::Initialize() const {
    return _impl->Initialize();
}

void Plugify::Terminate() const {
    _impl->Terminate();
}

bool Plugify::IsInitialized() const {
    return _impl->initialized;
}

void Plugify::Update(Duration deltaTime) const {
    _impl->Update(deltaTime);
}

const Manager& Plugify::GetManager() const noexcept {
    return _impl->manager;
}

const ServiceLocator& Plugify::GetServices() const noexcept {
    return _impl->services;
}

const Config& Plugify::GetConfig() const noexcept {
    return _impl->config;
}

const Version& Plugify::GetVersion() const noexcept {
    return _impl->version;
}

// PlugifyBuilder implementation
PlugifyBuilder& PlugifyBuilder::WithBaseDir(const std::filesystem::path& dir) {
    _config.paths.baseDir = dir;
    _config.paths.extensionsDir = dir / "extensions";
    _config.paths.configsDir = dir / "configs";
    _config.paths.dataDir = dir / "data";
    _config.paths.logsDir = dir / "logs";
    _config.paths.cacheDir = dir / "cache";
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithConfig(const Config& config) {
    _config = config;
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithConfig(Config&& config) {
    _config = std::move(config);
    return *this;
}

/*PlugifyBuilder& PlugifyBuilder::WithConfig(const std::filesystem::path& path) {
    _config = *glz::read_jsonc<Config>(*_services.Resolve<IFileSystem>()->ReadTextFile(path));
    return *this;
}*/

PlugifyBuilder& PlugifyBuilder::WithLogger(std::shared_ptr<ILogger> logger) {
    _services.RegisterInstance<ILogger>(std::move(logger));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithFileSystem(std::shared_ptr<IFileSystem> fs) {
    _services.RegisterInstance<IFileSystem>(std::move(fs));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader) {
    _services.RegisterInstance<IAssemblyLoader>(std::move(loader));
    return *this;
}

/*PlugifyBuilder& PlugifyBuilder::WithConfigProvider(std::shared_ptr<IConfigProvider> provider) {
    _services.RegisterInstance<IConfigProvider>(std::move(provider));
    return *this;
}*/

PlugifyBuilder& PlugifyBuilder::WithManifestParser(std::shared_ptr<IManifestParser> parser) {
    _services.RegisterInstance<IManifestParser>(std::move(parser));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithDependencyResolver(std::shared_ptr<IDependencyResolver> resolver) {
    _services.RegisterInstance<IDependencyResolver>(std::move(resolver));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithExtensionLifecycle(std::shared_ptr<IExtensionLifecycle> lifecycle) {
    _services.RegisterInstance<IExtensionLifecycle>(std::move(lifecycle));
    return *this;
}

/*PlugifyBuilder& PlugifyBuilder::WithProgressReporter(std::shared_ptr<IProgressReporter> reporter) {
    _services.RegisterInstance<IProgressReporter>(std::move(reporter));
    return *this;
}*/

/*PlugifyBuilder& PlugifyBuilder::WithMetricsCollector(std::shared_ptr<IMetricsCollector> metrics) {
    _services.RegisterInstance<IMetricsCollector>(std::move(metrics));
    return *this;
}*/

PlugifyBuilder& PlugifyBuilder::WithEventBus(std::shared_ptr<IEventBus> bus) {
    _services.RegisterInstance<IEventBus>(std::move(bus));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithDefaults() {
    _services.RegisterInstanceIfMissing<ILogger>(std::make_shared<ConsoleLogger>());
    _services.RegisterInstanceIfMissing<IPlatformOps>(CreatePlatformOps());
    _services.RegisterInstanceIfMissing<IEventBus>(std::make_shared<SimpleEventBus>());
    _services.RegisterInstanceIfMissing<IFileSystem>(std::make_shared<ExtendedFileSystem>());
    _services.RegisterInstanceIfMissing<IAssemblyLoader>(std::make_shared<BasicAssemblyLoader>(_services.Resolve<IPlatformOps>(), _services.Resolve<IFileSystem>()));
    //_services.RegisterInstanceIfMissing<IConfigProvider>(std::make_shared<TypedGlazeConfigProvider<Config>>());
    _services.RegisterInstanceIfMissing<IManifestParser>(std::make_shared<GlazeManifestParser>());
    _services.RegisterInstanceIfMissing<IDependencyResolver>(std::make_shared<LibsolvDependencyResolver>(_services.Resolve<ILogger>()));
    _services.RegisterInstanceIfMissing<IExtensionLifecycle>(std::make_shared<DummyLifecycle>());
    //_services.RegisterInstanceIfMissing<IProgressReporter>(std::make_shared<DefaultProgressReporter>(_services.Resolve<ILogger>()));
    //_services.RegisterInstanceIfMissing<IMetricsCollector>(std::make_shared<BasicMetricsCollector>());

    // Validate configuration
    if (_config.paths.baseDir.empty()) {
        _config.paths.baseDir = std::filesystem::current_path();
    }

    return *this;
}

std::shared_ptr<Plugify> PlugifyBuilder::Build() {
    // Ensure defaults are set
    WithDefaults();

    // Create Plugify instance
    auto plugify = std::shared_ptr<Plugify>(new Plugify(std::move(_services), std::move(_config)));

    return plugify;
}

PlugifyBuilder Plugify::CreateBuilder() {
    return PlugifyBuilder{};
}

// Convenience factory
Result<std::shared_ptr<Plugify>> plugify::MakePlugify(const std::filesystem::path& rootDir) {
    return Plugify::CreateBuilder()
        .WithBaseDir(rootDir)
        .Build();
}
