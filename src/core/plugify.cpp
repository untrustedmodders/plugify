#include <utility>

#include "plugify/asm/assembly_loader.hpp"
#include "plugify/core/plugify.hpp"
#include "core/console_logger.hpp"
#include "core/glaze_config_provider.hpp"
#include "core/glaze_manifest_parser.hpp"
#include "core/libsolv_dependency_resolver.hpp"
#include "core/standart_file_system.hpp"

using namespace plugify;

// Plugify::Impl
struct Plugify::Impl {
    Config config;
    Version version;
    std::shared_ptr<ServiceLocator> services;
    std::shared_ptr<Manager> manager;
    std::shared_ptr<Provider> provider;
    std::thread updateThread;
    std::atomic<bool> initialized{false};
    std::atomic<bool> shouldStop{false};

    Impl(Config cfg, ServiceLocator svc)
        : config(std::move(cfg))
        , services(std::make_shared<ServiceLocator>(std::move(svc))) {

        plg::parse(PLUGIFY_VERSION, version);

        // Create core components
        manager = std::make_shared<Manager>(services);
        services->Register<Manager>(manager);
        provider = std::make_shared<Provider>(services);
        services->Register<Provider>(provider);
    }

    ~Impl() {
        if (initialized) {
            Terminate();
        }
    }

    Result<void> Initialize() {
        if (initialized.exchange(true)) {
            return std::unexpected(Error{
                ErrorCode::InitializationFailed,
                "Already initialized"
            });
        }

        // Initialize logging
        LogInfo("Initializing Plugify...");

        // Create necessary directories
        if (!CreateDirectories()) {
            return std::unexpected("Failed to create directories");
        }

        // Load configuration
        if (auto configProvider = services->Get<IConfigProvider>()) {
            if (auto fileSystem = services->Get<IFileSystem>()) {
                auto configPath = config.paths.baseDir / config.paths.configsDir / "plugify.json";
                if (fileSystem->IsExists(configPath)) {
                    if (auto result = configProvider->LoadFromFile(configPath); !result) {
                        LogError(std::format("Failed to load config: {}", result.error()));
                        return std::unexpected("Failed to load config");
                    }
                }
            }
        }

        // Start update thread if configured
        if (config.runtime.updateInterval.count() > 0) {
            StartUpdateThread();
        }

        LogInfo("Plugify initialized successfully");
        LogInfo(std::format("Version: {}", version));
        LogInfo(std::format("Git: [{}]:({}) - {} on {} at '{}'", PLUGIFY_GIT_COMMIT_HASH, PLUGIFY_GIT_TAG, PLUGIFY_GIT_COMMIT_SUBJECT, PLUGIFY_GIT_BRANCH, PLUGIFY_GIT_COMMIT_DATE));
        LogInfo(std::format("Compiled on: {} from: {} with: '{}'", PLUGIFY_COMPILED_SYSTEM, PLUGIFY_COMPILED_GENERATOR, PLUGIFY_COMPILED_COMPILER));

        return {};
    }

    void Terminate() {
        if (!initialized.exchange(false)) {
            return;
        }

        LogInfo("Terminating Plugify...");

        // Stop update thread
        StopUpdateThread();

        // Unload all plugins
        if (manager) {
            if (auto result = manager->UnloadAllPlugins(); !result) {
                LogError(std::format("Failed to unload plugins: {}", result.error().message));
            }
        }

        // Save configuration
        if (auto configProvider = services->Get<IConfigProvider>()) {
            if (configProvider->IsDirty()) {
                auto configPath = config.paths.baseDir / config.paths.configsDir / "plugify.json";
                if (auto result = configProvider->SaveToFile(configPath); !result) {
                    LogError(std::format("Failed to save config: {}", result.error()));
                }
            }
        }

        // Flush logs
        if (auto logger = services->Get<ILogger>()) {
            logger->Log("Plugify terminated", Severity::Info);
            logger->Flush();
        }
    }

    void Update() {
        if (!initialized) return;

        if (manager) {
            manager->Update();
        }

        // Process events if event bus is available
        if (auto eventBus = services->Get<IEventBus>()) {
            // Event bus would process pending events here
        }
    }

private:
    bool CreateDirectories() {
        auto fileSystem = services->Get<IFileSystem>();
        if (!fileSystem) {
            return false;
        }

        if (!CheckDirectories()) {
            return false;
        }
        
        auto createDir = [&](const std::filesystem::path& dir) {
            if (!dir.empty() && !fileSystem->IsExists(dir)) {
                auto result = fileSystem->CreateDirectories(dir);
                if (!result) {
                    LogError(std::format("Failed to create directory '{}': {}", dir.string(), result.error()));
                } else {
                    LogInfo(std::format("Created directory: {}", dir.string()));
                }
            }
        };

        createDir(config.paths.baseDir);
        createDir(config.paths.baseDir / config.paths.pluginsDir);
        createDir(config.paths.baseDir / config.paths.configsDir);
        createDir(config.paths.baseDir / config.paths.dataDir);
        createDir(config.paths.baseDir / config.paths.logsDir);
        createDir(config.paths.baseDir / config.paths.cacheDir);
        return true;
    }

    bool CheckDirectories() {
        auto checkPath = [](const std::filesystem::path& p) {
            return !p.empty() && p.lexically_normal() == p;
        };

        std::vector<std::string> errors;

        if (!checkPath(config.paths.pluginsDir)) {
            errors.push_back(std::format("pluginsDir: - '{}'", config.paths.pluginsDir.string()));
        }
        if (!checkPath(config.paths.configsDir)) {
            errors.push_back(std::format("configsDir: - '{}'", config.paths.configsDir.string()));
        }
        if (!checkPath(config.paths.dataDir)) {
            errors.push_back(std::format("dataDir: - '{}'", config.paths.dataDir.string()));
        }
        if (!checkPath(config.paths.logsDir)) {
            errors.push_back(std::format("logsDir: '{}'", config.paths.logsDir.string()));
        }

        if (!errors.empty()) {
            LogError(std::format("{} path(s) must be relative", plg::join(errors, ", ")));
            return false;
        }

        std::array<std::filesystem::path, 5> dirs = {
                config.paths.pluginsDir,
                config.paths.configsDir,
                config.paths.dataDir,
                config.paths.logsDir,
        };

        auto pathCollides = [](const std::filesystem::path& first, const std::filesystem::path& second) {
            auto [itFirst, itSecond] = std::ranges::mismatch(first, second);
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
        shouldStop = false;
        updateThread = std::thread([this]() {
            while (!shouldStop) {
                Update();
                std::this_thread::sleep_for(config.runtime.updateInterval);
            }
        });
    }

    void StopUpdateThread() {
        shouldStop = true;
        if (updateThread.joinable()) {
            updateThread.join();
        }
    }

    void LogInfo(std::string_view msg) {
        if (auto logger = services->Get<ILogger>()) {
            logger->Log(msg, Severity::Info);
        }
    }

    void LogError(std::string_view msg) {
        if (auto logger = services->Get<ILogger>()) {
            logger->Log(msg, Severity::Error);
        }
    }
};

// Plugify implementation
Plugify::Plugify(Config config, ServiceLocator services)
    : _impl(std::make_unique<Impl>(std::move(config), std::move(services))) {
}

Plugify::~Plugify() = default;

Result<void> Plugify::Initialize() {
    return _impl->Initialize();
}

void Plugify::Terminate() {
    _impl->Terminate();
}

bool Plugify::IsInitialized() const noexcept {
    return _impl->initialized;
}

void Plugify::Update() {
    _impl->Update();
}

std::future<Result<void>> Plugify::InitializeAsync() {
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

const Config& Plugify::GetConfig() const {
    return _impl->config;
}

Version Plugify::GetVersion() const {
    return _impl->version;
}

std::shared_ptr<ServiceLocator> Plugify::GetServices() const {
    return _impl->services;
}

// PlugifyBuilder implementation
PlugifyBuilder& PlugifyBuilder::WithBaseDir(const std::filesystem::path& dir) {
    _config.paths.baseDir = dir;
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

PlugifyBuilder& PlugifyBuilder::WithLogger(std::shared_ptr<ILogger> logger) {
    _services.Register(std::move(logger));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithFileSystem(std::shared_ptr<IFileSystem> fs) {
    _services.Register(std::move(fs));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader) {
    _services.Register(std::move(loader));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithConfigProvider(std::shared_ptr<IConfigProvider> provider) {
    _services.Register(std::move(provider));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithManifestParser(std::shared_ptr<IManifestParser> parser) {
    _services.Register(std::move(parser));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithDependencyResolver(std::shared_ptr<IDependencyResolver> resolver) {
    _services.Register(std::move(resolver));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithPluginLifecycle(std::shared_ptr<IPluginLifecycle> lifecycle) {
    _services.Register(std::move(lifecycle));
    return *this;
}

PlugifyBuilder& PlugifyBuilder::WithEventBus(std::shared_ptr<IEventBus> bus) {
    _services.Register(std::move(bus));
    return *this;
}

Result<std::shared_ptr<Plugify>> PlugifyBuilder::Build() {
    // Provide default implementations if not specified
    if (!_services.Has<ILogger>()) {
        _services.Register<ILogger>(std::make_shared<ConsoleLogger>());
    }

    if (!_services.Has<IFileSystem>()) {
        _services.Register<IFileSystem>(std::make_shared<StandardFileSystem>());
    }

    if (!_services.Has<IAssemblyLoader>()) {
        _services.Register<IAssemblyLoader>(std::make_shared<AssemblyLoader>());
    }

    if (!_services.Has<IConfigProvider>()) {
        _services.Register<IConfigProvider>(std::make_shared<GlazeConfigProvider>());
    }

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

    // Create Plugify instance
    auto plugify = std::shared_ptr<Plugify>(
        new Plugify(std::move(_config), std::move(_services))
    );

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
