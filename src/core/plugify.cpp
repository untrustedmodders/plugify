#include <utility>

#include "plugify/asm/assembly_loader.hpp"
#include "plugify/core/plugify.hpp"

#include "core/console_logger.hpp"
#include "core/glaze_config_provider.hpp"
#include "core/glaze_manifest_parser.hpp"
#include "core/libsolv_dependency_resolver.hpp"
#include "core/standart_file_system.hpp"
#include "core/glaze_metadata.hpp"
#include "core/simple_event_bus.hpp"

using namespace plugify;

// Plugify::Impl
struct Plugify::Impl {
    std::shared_ptr<Context> context;
    std::shared_ptr<Manager> manager;
    Version version;
    std::jthread updateThread;
    std::atomic<bool> initialized{false};
    std::atomic<bool> shouldStop{false};

    Impl(std::shared_ptr<Context> ctx) : context(std::move(ctx)) {
        // Get version
        plg::parse(PLUGIFY_VERSION, version);

        // Create manager
        manager = std::make_shared<Manager>(context);
    }

    ~Impl() {
        if (initialized) {
            Terminate();
        }
    }

    Result<void> Initialize() {
        if (initialized) {
            return std::unexpected("Already initialized");
        }

        // Initialize logging
        LogInfo("Initializing Plugify...");

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
        manager->Initialize();

        // Start update thread if configured
        StartUpdateThread();

        LogInfo("Plugify initialized successfully");
        LogInfo(std::format("Version: {}", version));
        LogInfo(std::format("Git: [{}]:({}) - {} on {} at '{}'", PLUGIFY_GIT_COMMIT_HASH, PLUGIFY_GIT_TAG, PLUGIFY_GIT_COMMIT_SUBJECT, PLUGIFY_GIT_BRANCH, PLUGIFY_GIT_COMMIT_DATE));
        LogInfo(std::format("Compiled on: {} from: {} with: '{}'", PLUGIFY_COMPILED_SYSTEM, PLUGIFY_COMPILED_GENERATOR, PLUGIFY_COMPILED_COMPILER));

        initialized = true;

        // Publish initialization event
        if (auto bus = context->GetService<IEventBus>()) {
            bus->Publish("plugify.initialized", std::any{});
        }

        return {};
    }

    void Terminate() {
        if (!initialized) {
            return;
        }

        LogInfo("Terminating Plugify...");

        // Stop update thread
        StopUpdateThread();

        // Terminate manager
        manager->Terminate();

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
        if (auto bus = context->GetService<IEventBus>()) {
            bus->Publish("plugify.terminating", std::any{});
        }
        
        // Flush logs
        if (auto logger = context->GetService<ILogger>()) {
            logger->Log("Plugify terminated", Severity::Info);
            logger->Flush();
        }

        initialized = false;
    }

    void Update(std::chrono::microseconds deltaTime) {
        if (!initialized) return;

        manager->Update(deltaTime);

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

        auto fileSystem = context->GetService<IFileSystem>();
        if (!fileSystem) {
            return false;
        }

        auto createDir = [&](const std::filesystem::path& dir) {
            if (!dir.empty() && !fileSystem->IsExists(dir)) {
                if (auto result = fileSystem->CreateDirectories(dir); !result) {
                    LogError(result.error());
                } else {
                    LogInfo(std::format("Created directory: {}", dir.string()));
                }
            }
        };

        const auto& paths = context->GetConfig().paths;
        createDir(paths.baseDir);
        createDir(paths.baseDir / paths.extensionsDir);
        createDir(paths.baseDir / paths.configsDir);
        createDir(paths.baseDir / paths.dataDir);
        createDir(paths.baseDir / paths.logsDir);
        createDir(paths.baseDir / paths.cacheDir);
        return true;
    }

    bool CheckDirectories() {
        auto checkPath = [](const std::filesystem::path& p) {
            return !p.empty() && p.lexically_normal() == p;
        };

        std::vector<std::string> errors;

        const auto& paths = context->GetConfig().paths;
        
        if (!checkPath(paths.extensionsDir)) {
            errors.push_back(std::format("extensionsDir: {}", paths.extensionsDir.string()));
        }
        if (!checkPath(paths.configsDir)) {
            errors.push_back(std::format("configsDir: {}", paths.configsDir.string()));
        }
        if (!checkPath(paths.dataDir)) {
            errors.push_back(std::format("dataDir: {}", paths.dataDir.string()));
        }
        if (!checkPath(paths.logsDir)) {
            errors.push_back(std::format("logsDir: {}", paths.logsDir.string()));
        }
        if (!checkPath(paths.cacheDir)) {
            errors.push_back(std::format("cacheDir: {}", paths.cacheDir.string()));
        }

        if (!errors.empty()) {
            LogError(std::format("{} path(s) must be relative", plg::join(errors, ", ")));
            return false;
        }

        std::array<std::filesystem::path, 6> dirs = {
                paths.extensionsDir,
                paths.configsDir,
                paths.dataDir,
                paths.logsDir,
                paths.cacheDir,
        };

        auto pathCollides = [](const std::filesystem::path& first, const std::filesystem::path& second) {
            auto [itFirst, itSecond] = std::mismatch(first.begin(), first.end(), second.begin(), second.end());
            return itFirst == first.end() || itSecond == second.end();
        };

        for (auto first = dirs.begin(); first != dirs.end(); ++first) {
            for (auto second = first + 1; second != dirs.end(); ++second) {
                if (pathCollides(*first, *second)) {
                    errors.push_back(std::format("'{}' - '{}'", first->string(), second->string()));
                }
            }
        }

        if (!errors.empty()) {
            LogError(std::format("{} path(s) must be not collide between each other", plg::join(errors, ", ")));
            return false;
        }

        return true;
    }

    void StartUpdateThread() {
        const auto& runtime = context->GetConfig().runtime;
        if (runtime.updateInterval > Duration{0}) {
            updateThread = std::jthread([this, interval = runtime.updateInterval](std::stop_token token) {
                auto lastTime = Clock::now();

                while (!token.stop_requested()) {
                    auto now = Clock::now();
                    auto deltaTime = std::chrono::duration_cast<Duration>(now - lastTime);
                    lastTime = now;

                    Update(deltaTime);

                    std::this_thread::sleep_for(interval);
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

    void LogInfo(std::string_view msg) {
        if (auto logger = context->GetService<ILogger>()) {
            logger->Log(msg, Severity::Info);
        }
    }

    void LogError(std::string_view msg) {
        if (auto logger = context->GetService<ILogger>()) {
            logger->Log(msg, Severity::Error);
        }
    }
};

// Plugify implementation
Plugify::Plugify(std::shared_ptr<Context> context)
    : _impl(std::make_unique<Impl>(std::move(context))) {
}

Plugify::~Plugify() = default;

Result<void> Plugify::Initialize() {
    return _impl->Initialize();
}

void Plugify::Terminate() {
    _impl->Terminate();
}

bool Plugify::IsInitialized() const {
    return _impl->initialized;
}

void Plugify::Update(Duration deltaTime) {
    _impl->Update(deltaTime);
}

/*std::future<Result<void>> Plugify::InitializeAsync() {
    return std::async(std::launch::async, [this]() {
        return Initialize();
    });
}

std::future<void> Plugify::TerminateAsync() {
    return std::async(std::launch::async, [this]() {
        Terminate();
    });
}

std::shared_ptr<Manager> Plugify::GetManager() const {
    return _impl->manager;
}

std::shared_ptr<Provider> Plugify::GetProvider() const {
    return _impl->provider;
}

std::shared_ptr<Context> Plugify::GetContext() const {
    return _impl->context;
}

Version Plugify::GetVersion() const {
    return _impl->version;
}*/

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
    _config = *glz::read_jsonc<Config>(*_services.Get<IFileSystem>()->ReadTextFile(path));
    return *this;
}*/

PlugifyBuilder& PlugifyBuilder::WithLogger(std::shared_ptr<ILogger> logger) {
    _services.Register<ILogger>(std::move(logger));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithFileSystem(std::shared_ptr<IFileSystem> fs) {
    _services.Register<IFileSystem>(std::move(fs));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader) {
    _services.Register<IAssemblyLoader>(std::move(loader));
    return *this;
}

/*PlugifyBuilder& PlugifyBuilder::WithConfigProvider(std::shared_ptr<IConfigProvider> provider) {
    _services.Register<IConfigProvider>(std::move(provider));
    return *this;
}*/

PlugifyBuilder& PlugifyBuilder::WithManifestParser(std::shared_ptr<IManifestParser> parser) {
    _services.Register<IManifestParser>(std::move(parser));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithDependencyResolver(std::shared_ptr<IDependencyResolver> resolver) {
    _services.Register<IDependencyResolver>(std::move(resolver));
    return *this;
}

/*PlugifyBuilder& PlugifyBuilder::WithPluginLifecycle(std::shared_ptr<IPluginLifecycle> lifecycle) {
    _services.Register<IPluginLifecycle>(std::move(lifecycle));
    return *this;
}*/

PlugifyBuilder& PlugifyBuilder::WithEventBus(std::shared_ptr<IEventBus> bus) {
    _services.Register<IEventBus>(std::move(bus));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithDefaults() {
    // Provide default implementations if not specified
    if (!_services.Has<ILogger>()) {
        _services.Register<ILogger>(std::make_shared<ConsoleLogger>());
    }

    if (!_services.Has<IEventBus>()) {
        _services.Register<IEventBus>(std::make_shared<SimpleEventBus>());
    }

    if (!_services.Has<IFileSystem>()) {
        _services.Register<IFileSystem>(std::make_shared<ExtendedFileSystem>());
    }

    if (!_services.Has<IAssemblyLoader>()) {
        _services.Register<IAssemblyLoader>(std::make_shared<AssemblyLoader>());
    }

    /*if (!_services.Has<IConfigProvider>()) {
        _services.Register<IConfigProvider>(std::make_shared<TypedGlazeConfigProvider<Config>>());
    }*/

    if (!_services.Has<IManifestParser>()) {
        _services.Register<IManifestParser>(std::make_shared<GlazeManifestParser>());
    }

    if (!_services.Has<IDependencyResolver>()) {
        _services.Register<IDependencyResolver>(std::make_shared<LibsolvDependencyResolver>());
    }

    // Validate configuration
    if (_config.paths.baseDir.empty()) {
        _config.paths.baseDir = std::filesystem::current_path();
    }

    return *this;
}

std::shared_ptr<Plugify> PlugifyBuilder::Build() {
    // Ensure defaults are set
    WithDefaults();

    // Create context
    auto context = std::make_shared<Context>(
        std::move(_services),
        std::move(_config)
    );

    // Create Plugify instance
    auto plugify = std::shared_ptr<Plugify>(new Plugify(std::move(context)));

    return plugify;
}

PlugifyBuilder Plugify::CreateBuilder() {
    return PlugifyBuilder{};
}

// Convenience factory
Result<std::shared_ptr<Plugify>> MakePlugify(const std::filesystem::path& rootDir) {
    return Plugify::CreateBuilder()
        .WithBaseDir(rootDir)
        .Build();
}
