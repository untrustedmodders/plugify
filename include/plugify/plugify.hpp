#pragma once

#include "plugify/global.h"
#include "plugify/config.hpp"
#include "plugify/service_locator.hpp"
#include "plugify/manager.hpp"
#include "plugify/provider.hpp"
#include "plugify/config_provider.hpp"
#include "plugify/event_bus.hpp"
#include "plugify/dependency_resolver.hpp"
#include "plugify/file_system.hpp"
#include "plugify/manifest_parser.hpp"
#include "plugify/lifecycle.hpp"
//#include "plugify/progress_reporter.hpp"
//#include "plugify/metric_collector.hpp"
//#include "plugify/plugin_lifecycle.hpp"
#include "plugify/assembly_loader.hpp"

namespace std {
    class shared_ptr_access
    {
        template <typename _T, typename ... _Args>
        static _T* __construct(void* __pv, _Args&& ... __args)
        { return ::new(__pv) _T(forward<_Args>(__args)...); }

        template <typename _T>
        static void __destroy(_T* __ptr) { __ptr->~_T(); }

        template <typename _T, typename _A>
        friend class __shared_ptr_storage;
    };
}

namespace plugify {
    class Plugify;

	// Builder pattern for configuration
    class PLUGIFY_API PlugifyBuilder {
    public:
        PlugifyBuilder();
        ~PlugifyBuilder();
        PlugifyBuilder(const PlugifyBuilder& other) = delete;
        PlugifyBuilder(PlugifyBuilder&& other) noexcept = delete;
        PlugifyBuilder& operator=(const PlugifyBuilder& other) = delete;
        PlugifyBuilder& operator=(PlugifyBuilder&& other) noexcept = delete;

        // Path configuration methods - these have clear precedence
        PlugifyBuilder& WithBaseDir(std::filesystem::path dir);
        PlugifyBuilder& WithPaths(Config::Paths paths);

        // Config methods with clear semantics
        PlugifyBuilder& WithConfig(Config config);
        PlugifyBuilder& WithConfigFile(std::filesystem::path path);
        //PlugifyBuilder& WithPartialConfig(Config config); // Merges with existing

        // Explicit runtime configuration
        PlugifyBuilder& WithManualUpdate(); // Default
        PlugifyBuilder& WithBackgroundUpdate(std::chrono::milliseconds interval = std::chrono::milliseconds{16});
        PlugifyBuilder& WithUpdateCallback(std::function<void(std::chrono::milliseconds)> callback);

        // Service registration...
        PlugifyBuilder& WithLogger(std::shared_ptr<ILogger> logger);
        PlugifyBuilder& WithFileSystem(std::shared_ptr<IFileSystem> fs);
        PlugifyBuilder& WithAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader);
        //PlugifyBuilder& WithConfigProvider(std::shared_ptr<IConfigProvider> provider);
        PlugifyBuilder& WithManifestParser(std::shared_ptr<IManifestParser> parser);
        PlugifyBuilder& WithDependencyResolver(std::shared_ptr<IDependencyResolver> resolver);
        PlugifyBuilder& WithExtensionLifecycle(std::shared_ptr<IExtensionLifecycle> lifecycle);
        //PlugifyBuilder& WithProgressReporter(std::shared_ptr<IProgressReporter> reporter);
        //PlugifyBuilder& WithMetricsCollector(std::shared_ptr<IMetricsCollector> metrics);
        PlugifyBuilder& WithEventBus(std::shared_ptr<IEventBus> bus);

        PlugifyBuilder& WithDefaults();

        // Service registration with concepts for type safety
        template<typename Interface, typename Implementation>
            requires std::derived_from<Implementation, Interface>
        PlugifyBuilder& WithService(std::shared_ptr<Implementation> service) {
            GetServices().RegisterInstance<Interface>(std::move(service));
            return *this;
        }

        Result<std::shared_ptr<Plugify>> Build();

    PLUGIFY_ACCESS:
        const ServiceLocator& GetServices() const noexcept;
        Result<Config> LoadConfigFromFile(const std::filesystem::path& path) const;
        struct Impl;
        PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
    };

    class PLUGIFY_API Plugify {
    public:
        ~Plugify();
        Plugify(const Plugify& other) = delete;
        Plugify(Plugify&& other) noexcept = delete;
        Plugify& operator=(const Plugify& other) = delete;
        Plugify& operator=(Plugify&& other) noexcept = delete;

        // Lifecycle
        Result<void> Initialize() const;
        void Terminate() const;
        [[nodiscard]] bool IsInitialized() const;
        void Update(std::chrono::milliseconds deltaTime = std::chrono::milliseconds{16}) const;

        // Component access
        [[nodiscard]] const Manager& GetManager() const noexcept;
        [[nodiscard]] const ServiceLocator& GetServices() const noexcept;
        [[nodiscard]] const Config& GetConfig() const noexcept;
        [[nodiscard]] const Version& GetVersion() const noexcept;

        // Metrics and profiling
        /*struct Metrics {
            size_t loadedExtensions;
            size_t memoryUsageMB;
            double averageLoadTimeMs;
            double averageUpdateTimeMs;
        };
        [[nodiscard]] Metrics GetMetrics() const;*/

        // Factory method
        [[nodiscard]] static PlugifyBuilder CreateBuilder();

    PLUGIFY_ACCESS:
        explicit Plugify(ServiceLocator services, Config config);
        struct Impl;
        PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
    };

    // Convenience factory
    PLUGIFY_API Result<std::shared_ptr<Plugify>> MakePlugify(
        const std::filesystem::path& rootDir = std::filesystem::current_path());
}

