#include "plugify/assembly_loader.hpp"
#include "plugify/plugify.hpp"

#include "core/console_logger.hpp"
#include "core/glaze_manifest_parser.hpp"
#include "core/glaze_metadata.hpp"
#include "core/libsolv_dependency_resolver.hpp"
#include "core/standart_file_system.hpp"
#include "core/basic_assembly_loader.hpp"

using namespace plugify;

// Plugify::Impl
struct Plugify::Impl {
	ServiceLocator services;
	Config config;
	Manager manager;
	Version version;

	std::shared_ptr<ILogger> logger;
	std::shared_ptr<IFileSystem> fileSystem;
	std::shared_ptr<IPlatformOps> ops;
	std::shared_ptr<IProfiler> profiler;

	bool initialized{ false };

	std::thread::id ownerThreadId;
	std::chrono::steady_clock::time_point lastUpdateTime;

	Impl(ServiceLocator srv, Config cfg)
		: services(std::move(srv))
		, config(std::move(cfg))
		, manager(services, config)
		, ownerThreadId(std::this_thread::get_id()) {
		// Get version
		plg::parse(PLUGIFY_VERSION, version);

		// Create services
		logger = services.Resolve<ILogger>();
		fileSystem = services.Resolve<IFileSystem>();
		ops = services.Resolve<IPlatformOps>();
		profiler = services.TryResolve<IProfiler>();

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
		[[maybe_unused]] ScopedZone zone(profiler, {PLUGIFY_SIGNATURE});

		if (std::this_thread::get_id() != ownerThreadId) {
			return MakeError("Initialization must be called from owner thread");
		}

		if (initialized) {
			return MakeError("Already initialized");
		}

		logger->Log("Initializing Plugify...", Severity::Info);

		// Create necessary directories
		if (!CreateDirectories()) {
			return MakeError("Failed to create directories");
		}

		// Initialize manager
		//manager.Initialize();

		logger->Log("Plugify initialized successfully", Severity::Info);
		logger->Log(std::format("Version: {}", version), Severity::Info);

		initialized = true;

		return {};
	}

	void Terminate() {
		[[maybe_unused]] ScopedZone zone(profiler, {PLUGIFY_SIGNATURE});

		if (std::this_thread::get_id() != ownerThreadId) {
			throw std::runtime_error("Termination must be called from owner thread");
		}

		if (!initialized) {
			return;
		}

		logger->Log("Terminating Plugify...", Severity::Info);

		// Terminate manager
		manager.Terminate();

		logger->Log("Plugify terminated", Severity::Info);
		logger->Flush();

		initialized = false;
	}

	void Update(std::chrono::milliseconds deltaTime) {
		[[maybe_unused]] ScopedZone zone(profiler, {PLUGIFY_SIGNATURE});

		if (std::this_thread::get_id() != ownerThreadId) {
			throw std::runtime_error("Update must be called from owner thread");
		}

		if (!initialized) {
			return;
		}

		// Calculate delta time if not provided
		if (deltaTime == std::chrono::milliseconds{ 0 }) {
			auto now = std::chrono::steady_clock::now();
			deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime);
			lastUpdateTime = now;
		}

		// Direct update
		manager.Update(deltaTime);
	}

private:
	bool CreateDirectories() {
		if (!CheckDirectories()) {
			return false;
		}

		auto create_dir = [&](const std::filesystem::path& dir) {
			if (!dir.empty() && !fileSystem->IsExists(dir)) {
				if (auto result = fileSystem->CreateDirectories(dir); !result) {
					logger->Log(result.error(), Severity::Error);
				} else {
					logger->Log(
						std::format("Created directory: {}", plg::as_string(dir)),
						Severity::Info
					);
				}
			}
		};

		// createDir(config.paths.baseDir);
		create_dir(config.paths.extensionsDir);
		create_dir(config.paths.configsDir);
		create_dir(config.paths.dataDir);
		create_dir(config.paths.logsDir);
		create_dir(config.paths.cacheDir);
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
		auto check_collisions = [&result](std::span<const PathValidation> paths, size_t startIdx = 0) {
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
		check_collisions(pathsToValidate, 1);

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

// Service registration...
PlugifyBuilder& PlugifyBuilder::WithLogger(std::shared_ptr<ILogger> logger) {
	if (logger) {
		_impl->configOverrides.logging.severity = logger->GetLogLevel();
		_impl->services.RegisterInstance<ILogger>(std::move(logger));
	}
	return *this;
}
PlugifyBuilder& PlugifyBuilder::WithProfiler(std::shared_ptr<IProfiler> profiler) {
	if (profiler) _impl->services.RegisterInstance<IProfiler>(std::move(profiler));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithFileSystem(std::shared_ptr<IFileSystem> fs) {
	if (fs) _impl->services.RegisterInstance<IFileSystem>(std::move(fs));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader) {
	if (loader) _impl->services.RegisterInstance<IAssemblyLoader>(std::move(loader));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithManifestParser(std::shared_ptr<IManifestParser> parser) {
	if (parser) _impl->services.RegisterInstance<IManifestParser>(std::move(parser));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithDependencyResolver(std::shared_ptr<IDependencyResolver> resolver) {
	if (resolver) _impl->services.RegisterInstance<IDependencyResolver>(std::move(resolver));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithExtensionLifecycle(std::shared_ptr<IExtensionLifecycle> lifecycle) {
	if (lifecycle) _impl->services.RegisterInstance<IExtensionLifecycle>(std::move(lifecycle));
	return *this;
}

PlugifyBuilder& PlugifyBuilder::WithDefaults() {
	_impl->services.RegisterInstanceIfMissing<ILogger>(std::make_shared<ConsoleLogger>());
	//_impl->services.RegisterInstanceIfMissing<IProfiler>(std::make_shared<TracyProfiler>());
	_impl->services.RegisterInstanceIfMissing<IPlatformOps>(CreatePlatformOps());
	_impl->services.RegisterInstanceIfMissing<IFileSystem>(std::make_shared<ExtendedFileSystem>());
	_impl->services.RegisterInstanceIfMissing<IAssemblyLoader>(std::make_shared<BasicAssemblyLoader>(_impl->services.Resolve<IPlatformOps>(), _impl->services.Resolve<IFileSystem>()));
	_impl->services.RegisterInstanceIfMissing<IManifestParser>(std::make_shared<GlazeManifestParser>());
	_impl->services.RegisterInstanceIfMissing<IDependencyResolver>(std::make_shared<LibsolvDependencyResolver>(_impl->services.Resolve<ILogger>()));
	//_impl->services.RegisterInstanceIfMissing<IExtensionLifecycle>(std::make_shared<DummyLifecycle>());
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
