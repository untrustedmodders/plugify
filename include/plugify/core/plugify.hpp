#pragma once

#include "plugify/core/config.hpp"
#include "plugify/core/context.hpp"
#include "plugify/core/manager.hpp"
#include "plugify/core/provider.hpp"
#include "plugify/core/config_provider.hpp"
#include "plugify/core/event_bus.hpp"
#include "plugify/core/plugin_lifecycle.hpp"

#include "plugify_export.h"

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
        PlugifyBuilder& WithPluginLifecycle(std::shared_ptr<IPluginLifecycle> lifecycle);
        PlugifyBuilder& WithEventBus(std::shared_ptr<IEventBus> bus);

        PlugifyBuilder& WithDefaults();

        // Service registration with concepts for type safety
        template<typename Interface, typename Implementation>
            requires std::derived_from<Implementation, Interface>
        PlugifyBuilder& WithService(std::shared_ptr<Implementation> service) {
            _services.Register<Interface>(std::move(service));
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

        // Lifecycle
        Result<void> Initialize();
        void Terminate();
        [[nodiscard]] bool IsInitialized() const;
        void Update(double deltaTime = 0.016);

        // Async operations with coroutines (C++20)
        std::future<Result<void>> InitializeAsync();
        std::future<void> TerminateAsync();

        // Component access
        /*[[nodiscard]] std::shared_ptr<Manager> GetManager() const;
        [[nodiscard]] std::shared_ptr<Provider> GetProvider() const;
        [[nodiscard]] std::shared_ptr<Context> GetContext() const;
        [[nodiscard]] Version GetVersion() const;

        // Metrics and profiling
        struct Metrics {
            size_t loadedPackages;
            size_t memoryUsageMB;
            double averageLoadTimeMs;
            double averageUpdateTimeMs;
        };
        [[nodiscard]] Metrics GetMetrics() const;*/

        // Factory method
        [[nodiscard]] static PlugifyBuilder CreateBuilder();

    private:
        friend class PlugifyBuilder;
        explicit Plugify(std::shared_ptr<Context> context);

    private:
        std::unique_ptr<Impl> _impl;
    };

    // Convenience factory
    PLUGIFY_API Result<std::shared_ptr<Plugify>> MakePlugify(
        const std::filesystem::path& rootDir = std::filesystem::current_path());
}

