#pragma once

#include <memory>
#include <chrono>
#include <filesystem>
#include <functional>

#include "plugify/config.hpp"
#include "plugify/dependency_resolver.hpp"
#include "plugify/profiler.hpp"
#include "plugify/file_system.hpp"
#include "plugify/global.h"
#include "plugify/lifecycle.hpp"
#include "plugify/manager.hpp"
#include "plugify/provider.hpp"
#include "plugify/service_locator.hpp"
#include "plugify/assembly_loader.hpp"

namespace plugify {
	class Plugify;

	/**
	 * @class PlugifyBuilder
	 * @brief Assembles a @ref Plugify instance from a configuration and a set of services.
	 *
	 * Every service Plugify needs has a default implementation, so a caller only
	 * registers the ones it wants to replace. The With* methods return the builder
	 * itself, so they chain, and Build() hands back the finished instance.
	 *
	 * @code
	 * auto result = Plugify::CreateBuilder()
	 *                   .WithBaseDir("res")
	 *                   .WithLogger(myLogger)
	 *                   .WithDefaults()
	 *                   .Build();
	 * @endcode
	 *
	 * The class is a PIMPL wrapper, neither copyable nor movable.
	 *
	 * @see Plugify, MakePlugify
	 */
	class PLUGIFY_API PlugifyBuilder {
	public:
		/**
		 * @brief Construct a builder with the default configuration.
		 */
		PlugifyBuilder();

		/**
		 * @brief Destructor.
		 */
		~PlugifyBuilder();

		/**
		 * @brief Deleted copy constructor.
		 */
		PlugifyBuilder(const PlugifyBuilder& other) = delete;

		/**
		 * @brief Deleted move constructor.
		 */
		PlugifyBuilder(PlugifyBuilder&& other) noexcept = delete;

		/**
		 * @brief Deleted copy assignment operator.
		 */
		PlugifyBuilder& operator=(const PlugifyBuilder& other) = delete;

		/**
		 * @brief Deleted move assignment operator.
		 */
		PlugifyBuilder& operator=(PlugifyBuilder&& other) noexcept = delete;

		// Paths

		/**
		 * @brief Set the base directory every other path is relative to.
		 * @param dir The base directory.
		 * @return A reference to this builder for chaining.
		 */
		PlugifyBuilder& WithBaseDir(std::filesystem::path dir);

		/**
		 * @brief Set the whole directory layout at once.
		 * @param paths The directory layout.
		 * @return A reference to this builder for chaining.
		 */
		PlugifyBuilder& WithPaths(Config::Paths paths);

		// Config

		/**
		 * @brief Merge a configuration into the one being built.
		 * @param config The configuration to merge.
		 * @return A reference to this builder for chaining.
		 */
		PlugifyBuilder& WithConfig(Config config);

		/**
		 * @brief Merge a configuration file into the one being built.
		 * @param path Path of the configuration file.
		 * @return A reference to this builder for chaining.
		 * @note Values from a file are weaker than values set through the builder, so this never
		 * overwrites what the caller set explicitly. A file that cannot be read is reported from
		 * Build().
		 */
		PlugifyBuilder& WithConfigFile(std::filesystem::path path);

		// Services

		/**
		 * @brief Use a specific logger.
		 * @param logger The logger.
		 * @return A reference to this builder for chaining.
		 */
		PlugifyBuilder& WithLogger(std::shared_ptr<ILogger> logger);

		/**
		 * @brief Use a specific profiler.
		 * @param profiler The profiler.
		 * @return A reference to this builder for chaining.
		 */
		PlugifyBuilder& WithProfiler(std::shared_ptr<IProfiler> profiler);

		/**
		 * @brief Use a specific filesystem.
		 * @param fs The filesystem.
		 * @return A reference to this builder for chaining.
		 */
		PlugifyBuilder& WithFileSystem(std::shared_ptr<IFileSystem> fs);

		/**
		 * @brief Use a specific assembly loader.
		 * @param loader The assembly loader.
		 * @return A reference to this builder for chaining.
		 */
		PlugifyBuilder& WithAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader);

		/**
		 * @brief Use a specific dependency resolver.
		 * @param resolver The dependency resolver.
		 * @return A reference to this builder for chaining.
		 */
		PlugifyBuilder& WithDependencyResolver(std::shared_ptr<IDependencyResolver> resolver);

		/**
		 * @brief Observe the lifecycle of every extension.
		 * @param lifecycle The lifecycle listener.
		 * @return A reference to this builder for chaining.
		 */
		PlugifyBuilder& WithExtensionLifecycle(std::shared_ptr<IExtensionLifecycle> lifecycle);

		/**
		 * @brief Fill in a default implementation for every service still missing one.
		 * @return A reference to this builder for chaining.
		 * @note Services already registered are left alone, so this can be called at any point in
		 * the chain.
		 */
		PlugifyBuilder& WithDefaults();

		// Service registration with concepts for type safety

		/**
		 * @brief Register an arbitrary service.
		 * @tparam Interface The interface to register the service under.
		 * @tparam Implementation The concrete type, which has to derive from @p Interface.
		 * @param service The service instance.
		 * @return A reference to this builder for chaining.
		 */
		template <typename Interface, typename Implementation>
		    requires std::derived_from<Implementation, Interface>
		PlugifyBuilder& WithService(std::shared_ptr<Implementation> service) {
			GetServices().RegisterInstance<Interface>(std::move(service));
			return *this;
		}

		/**
		 * @brief Build the instance.
		 * @return The finished instance, or an error message if the configuration is invalid or a
		 * configuration file could not be read.
		 */
		Result<std::shared_ptr<Plugify>> Build();

		PLUGIFY_ACCESS :

		/**
		 * @brief Get the service container being built.
		 * @return The service container.
		 */
		const ServiceLocator& GetServices() const noexcept;

		/**
		 * @brief Read a configuration file.
		 * @param path Path of the configuration file.
		 * @return The parsed configuration, or an error message if it could not be read or parsed.
		 */
		Result<Config> LoadConfigFromFile(const std::filesystem::path& path) const;

		struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};

	/**
	 * @class Plugify
	 * @brief The root object of the framework.
	 *
	 * A Plugify instance holds the configuration, the service container and the
	 * @ref Manager that owns the extensions. It is created through
	 * @ref PlugifyBuilder (or the @ref MakePlugify shorthand), initialized once,
	 * updated every frame, and terminated at shutdown.
	 *
	 * The class is a PIMPL wrapper, neither copyable nor movable, and is held
	 * through a std::shared_ptr.
	 *
	 * @see PlugifyBuilder, Manager, Provider
	 */
	class PLUGIFY_API Plugify {
	public:
		/**
		 * @brief Destructor.
		 */
		~Plugify();

		/**
		 * @brief Deleted copy constructor.
		 */
		Plugify(const Plugify& other) = delete;

		/**
		 * @brief Deleted move constructor.
		 */
		Plugify(Plugify&& other) noexcept = delete;

		/**
		 * @brief Deleted copy assignment operator.
		 */
		Plugify& operator=(const Plugify& other) = delete;

		/**
		 * @brief Deleted move assignment operator.
		 */
		Plugify& operator=(Plugify&& other) noexcept = delete;

		// Lifecycle

		/**
		 * @brief Discover and bring up every extension.
		 * @return Nothing on success, an error message if the pipeline could not run.
		 * @see Manager::Initialize
		 */
		Result<void> Initialize() const;

		/**
		 * @brief Tear down every extension.
		 */
		void Terminate() const;

		/**
		 * @brief Check whether the instance has been initialized.
		 * @return True between a successful Initialize() and the next Terminate().
		 */
		[[nodiscard]] bool IsInitialized() const;

		/**
		 * @brief Call the update callback of every running extension.
		 * @param deltaTime Time since the previous update.
		 */
		void Update(std::chrono::milliseconds deltaTime = std::chrono::milliseconds{ 0 }) const;

		// Component access

		/**
		 * @brief Get the extension manager.
		 * @return The manager that owns every extension.
		 */
		[[nodiscard]] const Manager& GetManager() const noexcept;

		/**
		 * @brief Get the service container.
		 * @return The container every service was registered in.
		 */
		[[nodiscard]] const ServiceLocator& GetServices() const noexcept;

		/**
		 * @brief Get the effective configuration.
		 * @return The configuration this instance was built with.
		 */
		[[nodiscard]] const Config& GetConfig() const noexcept;

		/**
		 * @brief Get the version of the framework itself.
		 * @return The version of this Plugify build.
		 */
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

		/**
		 * @brief Start building an instance.
		 * @return A fresh builder.
		 */
		[[nodiscard]] static PlugifyBuilder CreateBuilder();

		PLUGIFY_ACCESS :

		/**
		 * @brief Construct an instance from a finished configuration.
		 * @param services The service container.
		 * @param config The configuration.
		 * @note Called by PlugifyBuilder::Build; use the builder instead.
		 */
		explicit Plugify(ServiceLocator services, Config config);

		struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};

	/**
	 * @brief Build an instance with the default services.
	 * @param rootDir Base directory for the instance; defaults to the current working directory.
	 * @return The finished instance, or an error message if it could not be built.
	 * @note Shorthand for a builder with @ref PlugifyBuilder::WithBaseDir and
	 * @ref PlugifyBuilder::WithDefaults.
	 */
	PLUGIFY_API Result<std::shared_ptr<Plugify>>
	MakePlugify(const std::filesystem::path& rootDir = std::filesystem::current_path());
}
