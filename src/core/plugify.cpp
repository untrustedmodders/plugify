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
using namespace std::chrono_literals;

// Plugify::Impl
struct Plugify::Impl {
	ServiceLocator services;
	Config config;
	Manager manager;
	Version version;

	std::shared_ptr<ILogger> logger;
	std::shared_ptr<IEventBus> eventBus;
	std::shared_ptr<IFileSystem> fileSystem;
	std::shared_ptr<IPlatformOps> ops;

	bool initialized{ false };

	std::thread::id ownerThreadId;
	std::chrono::steady_clock::time_point lastUpdateTime;
	// TODO: replace by jthread when it will be available on mac
	std::thread updateThread;
	std::mutex cvMutex;
	std::condition_variable cv;
	std::atomic<bool> stopRequested{ false };

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
		ops = services.Resolve<IPlatformOps>();

		// Set default logging level
		logger->SetLogLevel(config.logging.severity);

		// Initialize update time
		lastUpdateTime = std::chrono::steady_clock::now();
	}

	~Impl() {
		if (initialized) {
			Terminate();
		}
	}

	Result<void> Initialize() {
		// Thread safety check based on config
		if (config.runtime.pinToMainThread && std::this_thread::get_id() != ownerThreadId) {
			return MakeError("Initialization must be called from owner thread");
		}

		if (initialized) {
			return MakeError("Already initialized");
		}

		logger->Log("Initializing Plugify...", Severity::Info);
		logger->Log(
			std::format("Update mode: {}", plg::enum_to_string(config.runtime.updateMode)),
			Severity::Info
		);

		// Create necessary directories
		if (!CreateDirectories()) {
			return MakeError("Failed to create directories");
		}

		// Initialize manager
		// manager.Initialize();

		// Start update mechanism based on mode
		if (auto result = StartUpdateMechanism(); !result) {
			return result;
		}

		logger->Log("Plugify initialized successfully", Severity::Info);
		logger->Log(std::format("Version: {}", version), Severity::Info);

		initialized = true;

		// Publish initialization event
		eventBus->Publish("plugify.initialized", std::any{});

		return {};
	}

	void Terminate() {
		if (config.runtime.pinToMainThread && std::this_thread::get_id() != ownerThreadId) {
			throw std::runtime_error("Termination must be called from owner thread");
		}

		if (!initialized) {
			return;
		}

		logger->Log("Terminating Plugify...", Severity::Info);

		// Stop update mechanism
		StopUpdateMechanism();

		// Terminate manager
		manager.Terminate();

		// Publish termination event
		eventBus->Publish("plugify.terminating", std::any{});

		logger->Log("Plugify terminated", Severity::Info);
		logger->Flush();

		initialized = false;
	}

	void Update(std::chrono::milliseconds deltaTime = 0ms) {
		if (config.runtime.pinToMainThread && std::this_thread::get_id() != ownerThreadId) {
			throw std::runtime_error("Update must be called from owner thread");
		}

		if (!initialized) {
			return;
		}

		// Calculate delta time if not provided
		if (deltaTime == 0ms) {
			auto now = std::chrono::steady_clock::now();
			deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime);
			lastUpdateTime = now;
		}

		// Update based on mode
		switch (config.runtime.updateMode) {
			case UpdateMode::Manual:
				// Direct update
				manager.Update(deltaTime);
				break;

			case UpdateMode::Callback:
				// Call user callback which should eventually call manager.Update
				if (config.runtime.updateCallback) {
					config.runtime.updateCallback(deltaTime);
				}
				break;

			case UpdateMode::BackgroundThread:
				// Background thread handles updates, this call is a no-op
				logger->Log("Update() called in BackgroundThread mode - ignoring", Severity::Debug);
				break;
		}

		// Process any pending events
		// eventBus->ProcessPending();
	}

	// Get current update statistics
	/*struct UpdateStats {
		UpdateMode mode;
		size_t updateCount;
		std::chrono::milliseconds averageUpdateTime;
		std::chrono::milliseconds lastUpdateDuration;
		bool isBackgroundThreadRunning;
	};

	UpdateStats GetUpdateStats() const {
		return {
			.mode = config.runtime.updateMode,
			.updateCount = manager.GetUpdateCount(),
			.averageUpdateTime = manager.GetAverageUpdateTime(),
			.lastUpdateDuration = manager.GetLastUpdateDuration(),
			.isBackgroundThreadRunning = updateThread.joinable()
		};
	}*/

private:
	bool CreateDirectories() {
		if (!CheckDirectories()) {
			return false;
		}

		auto createDir = [&](const std::filesystem::path& dir) {
			if (!dir.empty() && !fileSystem->IsExists(dir)) {
				if (auto result = fileSystem->CreateDirectories(dir); !result) {
					logger->Log(result.error(), Severity::Error);
					;
				} else {
					logger->Log(
						std::format("Created directory: {}", plg::as_string(dir)),
						Severity::Info
					);
				}
			}
		};

		// createDir(config.paths.baseDir);
		createDir(config.paths.extensionsDir);
		createDir(config.paths.configsDir);
		createDir(config.paths.dataDir);
		createDir(config.paths.logsDir);
		createDir(config.paths.cacheDir);
		return true;
	}

	bool CheckDirectories() {
		struct PathValidation {
			std::string_view name;
			const std::filesystem::path* path;
			bool requireExists;	   // Optional: check if directory should exist
			bool requireRelative;  // Optional: check if path should be relative
			bool canCreate;		   // Optional: whether we can create it if missing
		};

		const std::array<PathValidation, 6> pathsToValidate = {
			PathValidation{ "baseDir", &config.paths.baseDir, true, false, false },
			PathValidation{ "extensionsDir", &config.paths.extensionsDir, false, false, true },
			PathValidation{ "configsDir", &config.paths.configsDir, false, false, true },
			PathValidation{ "dataDir", &config.paths.dataDir, false, false, true },
			PathValidation{ "logsDir", &config.paths.logsDir, false, false, true },
			PathValidation{ "cacheDir", &config.paths.cacheDir, false, false, true }
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
			if (path->empty()) {
				result.AddError(std::format("{} is not configured", name));
				continue;
			}

			// Check if path is normalized
			if (path->lexically_normal() != *path) {
				result.AddError(std::format("{} is not normalized: '{}'", name, plg::as_string(*path)));
				continue;
			}

			// Check if path is relative (optional, depending on requirements)
			if (requireRelative && path->is_absolute()) {
				result.AddError(std::format("{} must be relative: '{}'", name, plg::as_string(*path)));
				continue;
			}

			// Check existence if required
			bool exists = fileSystem->IsExists(*path);

			if (requireExists && !exists) {
				result.AddError(std::format("{} does not exist: '{}'", name, plg::as_string(*path)));
			} else if (!exists && canCreate) {
				result.AddWarning(std::format("{} will be created: '{}'", name, plg::as_string(*path)));
			}
		}

		// Step 2: Check for path collisions
		auto checkCollisions = [&result](std::span<const PathValidation> paths, size_t startIdx = 0) {
			for (size_t i = startIdx; i < paths.size(); ++i) {
				if (paths[i].path->empty()) {
					continue;
				}

				for (size_t j = i + 1; j < paths.size(); ++j) {
					if (paths[j].path->empty()) {
						continue;
					}

					const auto& path1 = *paths[i].path;
					const auto& path2 = *paths[j].path;

					// Check if one path is a parent of another
					auto [it1, it2] = std::mismatch(
						path1.begin(),
						path1.end(),
						path2.begin(),
						path2.end()
					);

					if (it1 == path1.end() || it2 == path2.end()) {
						result.AddError(
							std::format(
								"{} ('{}') conflicts with {} ('{}')",
								paths[i].name,
								plg::as_string(path1),
								paths[j].name,
								plg::as_string(path2)
							)
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
			logger->Log(
				std::format("Validation failed: {}", plg::join(result.errors, "; ")),
				Severity::Error
			);
		}

		return result.IsSuccess();
	}

	Result<void> StartUpdateMechanism() {
		switch (config.runtime.updateMode) {
			case UpdateMode::Manual:
				// Nothing to start for manual mode
				logger->Log("Manual update mode - call Update() explicitly", Severity::Info);
				break;

			case UpdateMode::BackgroundThread:
				if (config.runtime.updateInterval <= 0ms) {
					return MakeError("Invalid update interval for background thread");
				}
				StartBackgroundUpdateThread();
				break;

			case UpdateMode::Callback:
				if (!config.runtime.updateCallback) {
					return MakeError("No update callback provided");
				}
				logger->Log("Callback update mode configured", Severity::Info);
				break;
		}

		return {};
	}

	void StopUpdateMechanism() {
		switch (config.runtime.updateMode) {
			case UpdateMode::BackgroundThread:
				StopBackgroundUpdateThread();
				break;
			default:
				// Nothing to stop for other modes
				break;
		}
	}

	void StartBackgroundUpdateThread() {
		stopRequested = false;

		updateThread = std::thread([this]() {
			// Set thread priority if specified
			/*if (config.runtime.threadPriority) {
				ops->SetThreadPriority(*config.runtime.threadPriority);
			}*/

			auto lastTime = std::chrono::steady_clock::now();
			const auto& interval = config.runtime.updateInterval;

			logger->Log(
				std::format("Background update thread started (interval: {})", interval),
				Severity::Info
			);

			while (!stopRequested.load(std::memory_order_relaxed)) {
				auto now = std::chrono::steady_clock::now();
				auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime);
				lastTime = now;

				// Perform update
				manager.Update(deltaTime);

				// Sleep for remaining time
				auto updateDuration = std::chrono::steady_clock::now() - now;
				auto sleepTime = interval
								 - std::chrono::duration_cast<std::chrono::milliseconds>(updateDuration
								 );

				if (sleepTime > 0ms) {
					// Use interruptible sleep
					std::unique_lock lock(cvMutex);
					cv.wait_for(lock, sleepTime, [this] {
						return stopRequested.load(std::memory_order_relaxed);
					});
				} else {
					logger->Log(
						std::format(
							"Update took longer than interval: {} > {}",
							std::chrono::duration_cast<std::chrono::milliseconds>(updateDuration),
							interval
						),
						Severity::Warning
					);
				}
			}

			logger->Log("Background update thread stopped", Severity::Info);
		});
	}

	void StopBackgroundUpdateThread() {
		stopRequested.store(true, std::memory_order_relaxed);
		cv.notify_all();  // Wake up the thread if it's sleeping

		if (updateThread.joinable()) {
			logger->Log("Stopping background update thread...", Severity::Info);
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

void Plugify::Update(std::chrono::milliseconds deltaTime) const {
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

struct PlugifyBuilder::Impl {
	ServiceLocator services;
	Config baseConfig;
	Config configOverrides;
	std::optional<std::filesystem::path> configFilePath;
	bool hasBaseConfig = false;
	bool hasBaseDirOverride = false;
	bool hasPathsOverride = false;
};

// Plugify implementation
PlugifyBuilder::PlugifyBuilder()
	: _impl(std::make_unique<Impl>()) {
}

PlugifyBuilder::~PlugifyBuilder() = default;

// Path configuration methods
PlugifyBuilder& PlugifyBuilder::WithBaseDir(std::filesystem::path dir) {
	_impl->configOverrides.paths.baseDir = std::move(dir);
	_impl->hasBaseDirOverride = true;
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithPaths(Config::Paths paths) {
	_impl->configOverrides.paths = std::move(paths);
	_impl->hasPathsOverride = true;
	return *this;
}

// Config loading methods with clear priority
PlugifyBuilder& PlugifyBuilder::WithConfigFile(std::filesystem::path path) {
	_impl->configFilePath = std::move(path);
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithConfig(Config config) {
	_impl->baseConfig = std::move(config);
	_impl->hasBaseConfig = true;
	return *this;
}

// Update mode configuration
PlugifyBuilder& PlugifyBuilder::WithManualUpdate() {
	_impl->configOverrides.runtime.updateMode = UpdateMode::Manual;
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithBackgroundUpdate(std::chrono::milliseconds interval) {
	_impl->configOverrides.runtime.updateMode = UpdateMode::BackgroundThread;
	_impl->configOverrides.runtime.updateInterval = interval;
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithUpdateCallback(
	std::function<void(std::chrono::milliseconds)> callback
) {
	_impl->configOverrides.runtime.updateMode = UpdateMode::Callback;
	_impl->configOverrides.runtime.updateCallback = std::move(callback);
	return *this;
}

// Service registration...
PlugifyBuilder& PlugifyBuilder::WithLogger(std::shared_ptr<ILogger> logger) {
	_impl->services.RegisterInstance<ILogger>(std::move(logger));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithFileSystem(std::shared_ptr<IFileSystem> fs) {
	_impl->services.RegisterInstance<IFileSystem>(std::move(fs));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader) {
	_impl->services.RegisterInstance<IAssemblyLoader>(std::move(loader));
	return *this;
}

/*PlugifyBuilder& PlugifyBuilder::WithConfigProvider(std::shared_ptr<IConfigProvider> provider) {
	_impl->services.RegisterInstance<IConfigProvider>(std::move(provider));
	return *this;
}*/

PlugifyBuilder& PlugifyBuilder::WithManifestParser(std::shared_ptr<IManifestParser> parser) {
	_impl->services.RegisterInstance<IManifestParser>(std::move(parser));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithDependencyResolver(std::shared_ptr<IDependencyResolver> resolver) {
	_impl->services.RegisterInstance<IDependencyResolver>(std::move(resolver));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithExtensionLifecycle(std::shared_ptr<IExtensionLifecycle> lifecycle) {
	_impl->services.RegisterInstance<IExtensionLifecycle>(std::move(lifecycle));
	return *this;
}

/*PlugifyBuilder& PlugifyBuilder::WithProgressReporter(std::shared_ptr<IProgressReporter> reporter)
{ _impl->services.RegisterInstance<IProgressReporter>(std::move(reporter)); return *this;
}*/

/*PlugifyBuilder& PlugifyBuilder::WithMetricsCollector(std::shared_ptr<IMetricsCollector> metrics) {
	_impl->services.RegisterInstance<IMetricsCollector>(std::move(metrics));
	return *this;
}*/

PlugifyBuilder& PlugifyBuilder::WithEventBus(std::shared_ptr<IEventBus> bus) {
	_impl->services.RegisterInstance<IEventBus>(std::move(bus));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithDefaults() {
	_impl->services.RegisterInstanceIfMissing<ILogger>(std::make_shared<ConsoleLogger>());
	_impl->services.RegisterInstanceIfMissing<IPlatformOps>(CreatePlatformOps());
	_impl->services.RegisterInstanceIfMissing<IEventBus>(std::make_shared<SimpleEventBus>());
	_impl->services.RegisterInstanceIfMissing<IFileSystem>(std::make_shared<ExtendedFileSystem>());
	_impl->services.RegisterInstanceIfMissing<IAssemblyLoader>(std::make_shared<BasicAssemblyLoader>(
		_impl->services.Resolve<IPlatformOps>(),
		_impl->services.Resolve<IFileSystem>()
	));
	//_impl->services.RegisterInstanceIfMissing<IConfigProvider>(std::make_shared<TypedGlazeConfigProvider<Config>>());
	_impl->services.RegisterInstanceIfMissing<IManifestParser>(std::make_shared<GlazeManifestParser>()
	);
	_impl->services.RegisterInstanceIfMissing<IDependencyResolver>(
		std::make_shared<LibsolvDependencyResolver>(_impl->services.Resolve<ILogger>())
	);
	_impl->services.RegisterInstanceIfMissing<IExtensionLifecycle>(std::make_shared<DummyLifecycle>());
	//_impl->services.RegisterInstanceIfMissing<IProgressReporter>(std::make_shared<DefaultProgressReporter>(_impl->services.Resolve<ILogger>()));
	//_impl->services.RegisterInstanceIfMissing<IMetricsCollector>(std::make_shared<BasicMetricsCollector>());
	return *this;
}

Result<std::shared_ptr<Plugify>> PlugifyBuilder::Build() {
	// Apply configuration resolution strategy:
	// 1. Start with defaults
	Config finalConfig;

	// 2. Load from file if specified
	if (_impl->configFilePath) {
		if (auto fileConfig = LoadConfigFromFile(*_impl->configFilePath)) {
			finalConfig.MergeFrom(*fileConfig, ConfigSource::File);
		} else {
			return MakeError("Failed to load config from {}", _impl->configFilePath->string());
		}
	}

	// 3. Apply base config if provided
	if (_impl->hasBaseConfig) {
		finalConfig.MergeFrom(_impl->baseConfig, ConfigSource::Builder);
	}

	// 4. Apply specific overrides (highest priority)
	if (_impl->hasBaseDirOverride || _impl->hasPathsOverride) {
		finalConfig.MergeFrom(_impl->configOverrides, ConfigSource::Override);
	}

	// 5. Ensure base directory is set
	if (finalConfig.paths.baseDir.empty()) {
		finalConfig.paths.baseDir = std::filesystem::current_path();
	}

	// 6. Resolve all relative paths
	finalConfig.paths.ResolveRelativePaths();

	// 7. Validate final configuration
	if (auto result = finalConfig.Validate(); !result) {
		return MakeError("Configuration validation failed: {}", result.error());
	}

	// 8. Set up services with defaults
	WithDefaults();

	// 9. Create Plugify instance
	return std::make_shared<Plugify>(std::move(_impl->services), std::move(finalConfig));
}

const ServiceLocator& PlugifyBuilder::GetServices() const noexcept {
	return _impl->services;
}

Result<Config> PlugifyBuilder::LoadConfigFromFile(const std::filesystem::path& path) const {
	auto fs = _impl->services.TryResolve<IFileSystem>();
	if (!fs) {
		fs = std::make_shared<StandardFileSystem>();
	}

	auto text = fs->ReadTextFile(path);
	if (!text) {
		return MakeError("Failed to read config file");
	}

	auto config = glz::read_jsonc<Config>(*text);
	if (!config) {
		return MakeError("Failed to parse config file");
	}

	return *config;
}

PlugifyBuilder Plugify::CreateBuilder() {
	return PlugifyBuilder{};
}

// Convenience factory
Result<std::shared_ptr<Plugify>> plugify::MakePlugify(const std::filesystem::path& rootDir) {
	return Plugify::CreateBuilder().WithBaseDir(rootDir).Build();
}
