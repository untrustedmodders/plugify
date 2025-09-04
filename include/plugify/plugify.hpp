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
//#include "plugify/progress_reporter.hpp"
//#include "plugify/metric_collector.hpp"
//#include "plugify/plugin_lifecycle.hpp"
#include "plugify/assembly_loader.hpp"

namespace plugify {
    class Plugify;

	// Builder pattern for configuration
    class PLUGIFY_API PlugifyBuilder {
    public:
        PlugifyBuilder& WithBaseDir(const std::filesystem::path& dir);
        PlugifyBuilder& WithConfig(const Config& config);
        PlugifyBuilder& WithConfig(Config&& config);
        PlugifyBuilder& WithLogger(std::shared_ptr<ILogger> logger);
        PlugifyBuilder& WithFileSystem(std::shared_ptr<IFileSystem> fs);
        PlugifyBuilder& WithAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader);
        //PlugifyBuilder& WithConfigProvider(std::shared_ptr<IConfigProvider> provider);
        PlugifyBuilder& WithManifestParser(std::shared_ptr<IManifestParser> parser);
        PlugifyBuilder& WithDependencyResolver(std::shared_ptr<IDependencyResolver> resolver);
        //PlugifyBuilder& WithPluginLifecycle(std::shared_ptr<IPluginLifecycle> lifecycle);
        //PlugifyBuilder& WithProgressReporter(std::shared_ptr<IProgressReporter> reporter);
        //PlugifyBuilder& WithMetricsCollector(std::shared_ptr<IMetricsCollector> metrics);
        PlugifyBuilder& WithEventBus(std::shared_ptr<IEventBus> bus);

        PlugifyBuilder& WithDefaults();

        // Service registration with concepts for type safety
        template<typename Interface, typename Implementation>
            requires std::derived_from<Implementation, Interface>
        PlugifyBuilder& WithService(std::shared_ptr<Implementation> service) {
            _services.RegisterInstance<Interface>(std::move(service));
            return *this;
        }

        std::shared_ptr<Plugify> Build();

    private:
        ServiceLocator _services;
        Config _config;
    };

    class PLUGIFY_API Plugify {
        struct Impl;
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
        void Update(Duration deltaTime = Duration{16}) const;

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

    private:
        friend class PlugifyBuilder;
        explicit Plugify(ServiceLocator services, Config config);

    private:
        PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
    };

    // Convenience factory
    PLUGIFY_API Result<std::shared_ptr<Plugify>> MakePlugify(
        const std::filesystem::path& rootDir = std::filesystem::current_path());
}

