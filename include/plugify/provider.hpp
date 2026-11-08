#pragma once

#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <optional>

#include "plugify/global.h"
#include "plugify/logger.hpp"
#include "plugify/service_locator.hpp"
#include "plugify/types.hpp"

namespace plugify {
	struct Config;
	class Manager;
	class Extension;

	/**
	 * @class Provider
	 * @brief The view of the framework that an extension gets.
	 *
	 * A provider is a facade over the logger, the configured paths, the
	 * @ref Manager and the service container, so a language module does not have
	 * to reach for any of them itself. Every language module receives one in
	 * ILanguageModule::Initialize, and it stays valid for as long as the module is
	 * loaded.
	 *
	 * The class is a PIMPL wrapper and is copyable; copies refer to the same
	 * framework.
	 *
	 * @see ILanguageModule, Manager, ServiceLocator
	 */
	class PLUGIFY_API Provider {
	public:
		/**
		 * @brief Construct a provider over a framework instance.
		 * @param services The service container.
		 * @param config The effective configuration.
		 * @param manager The extension manager.
		 * @note All three references have to outlive the provider.
		 */
		Provider(const ServiceLocator& services, const Config& config, const Manager& manager);

		/**
		 * @brief Destructor.
		 */
		~Provider();

		/**
		 * @brief Copy constructor.
		 * @param other Another instance of Provider.
		 */
		Provider(const Provider& other);

		/**
		 * @brief Move constructor.
		 * @param other Another instance of Provider.
		 */
		Provider(Provider&& other) noexcept;

		/**
		 * @brief Copy assignment operator.
		 * @param other Another instance of Provider.
		 * @return A reference to this instance after copying.
		 */
		Provider& operator=(const Provider& other);

		/**
		 * @brief Move assignment operator.
		 * @param other Another instance of Provider.
		 * @return A reference to this instance after moving.
		 */
		Provider& operator=(Provider&& other) noexcept;

		// Logging helpers

		/**
		 * @brief Log a message through the logger of the framework.
		 * @param msg The message.
		 * @param sev Severity of the message.
		 * @param loc Source location the message came from. Defaults to the current location.
		 */
		void Log(
		    std::string_view msg,
		    Severity sev = Severity::Info,
		    const Location& loc = Location::current()
		) const;

		/**
		 * @brief Log a message with @ref Severity::Trace.
		 * @param msg The message.
		 * @param loc Source location the message came from.
		 */
		void LogTrace(
		    std::string_view msg,
		    const Location& loc = Location::current()
		) const {
			Log(msg, Severity::Trace, loc);
		}

		/**
		 * @brief Log a message with @ref Severity::Debug.
		 * @param msg The message.
		 * @param loc Source location the message came from.
		 */
		void LogDebug(
		    std::string_view msg,
		    const Location& loc = Location::current()
		) const {
			Log(msg, Severity::Debug, loc);
		}

		/**
		 * @brief Log a message with @ref Severity::Info.
		 * @param msg The message.
		 * @param loc Source location the message came from.
		 */
		void LogInfo(
		    std::string_view msg,
		    const Location& loc = Location::current()
		) const {
			Log(msg, Severity::Info, loc);
		}

		/**
		 * @brief Log a message with @ref Severity::Warning.
		 * @param msg The message.
		 * @param loc Source location the message came from.
		 */
		void LogWarning(
		    std::string_view msg,
		    const Location& loc = Location::current()
		) const {
			Log(msg, Severity::Warning, loc);
		}

		/**
		 * @brief Log a message with @ref Severity::Error.
		 * @param msg The message.
		 * @param loc Source location the message came from.
		 */
		void LogError(
		    std::string_view msg,
		    const Location& loc = Location::current()
		) const {
			Log(msg, Severity::Error, loc);
		}

		/**
		 * @brief Log a message with @ref Severity::Fatal.
		 * @param msg The message.
		 * @param loc Source location the message came from.
		 */
		void LogFatal(
		    std::string_view msg,
		    const Location& loc = Location::current()
		) const {
			Log(msg, Severity::Fatal, loc);
		}

		/**
		 * @brief Check whether modules should prefer their own symbols.
		 * @return The configured value of Config::Loading::preferOwnSymbols.
		 */
		bool IsPreferOwnSymbols() const noexcept;

		// Path helpers

		/**
		 * @brief Get the base directory.
		 * @return Absolute path every other directory is relative to.
		 */
		[[nodiscard]] const std::filesystem::path& GetBaseDir() const noexcept;

		/**
		 * @brief Get the extensions directory.
		 * @return Absolute path of the folder extensions are discovered in.
		 */
		[[nodiscard]] const std::filesystem::path& GetExtensionsDir() const noexcept;

		/**
		 * @brief Get the configs directory.
		 * @return Absolute path of the folder extensions keep their configuration in.
		 */
		[[nodiscard]] const std::filesystem::path& GetConfigsDir() const noexcept;

		/**
		 * @brief Get the data directory.
		 * @return Absolute path of the folder extensions keep their data in.
		 */
		[[nodiscard]] const std::filesystem::path& GetDataDir() const noexcept;

		/**
		 * @brief Get the logs directory.
		 * @return Absolute path of the folder logs are written to.
		 */
		[[nodiscard]] const std::filesystem::path& GetLogsDir() const noexcept;

		/**
		 * @brief Get the cache directory.
		 * @return Absolute path of the folder cached files are written to.
		 */
		[[nodiscard]] const std::filesystem::path& GetCacheDir() const noexcept;

		// Manager

		/**
		 * @brief Check whether an extension is loaded and in range.
		 * @param name Name of the extension.
		 * @param constraint Version constraints the extension has to satisfy; omit to accept any
		 * version.
		 * @return True if the extension is running and its version matches.
		 * @see Manager::IsExtensionLoaded
		 */
		[[nodiscard]] bool
		IsExtensionLoaded(std::string_view name, std::optional<Constraint> constraint = {}) const noexcept;

		/**
		 * @brief Look an extension up by name.
		 * @param name Name of the extension, as written in its manifest.
		 * @return The extension, or nullptr if no running extension has that name.
		 * @note The pointer points into the list of the manager and stays valid until the next
		 * Manager::Initialize rebuilds it.
		 */
		[[nodiscard]] const Extension* FindExtension(std::string_view name) const noexcept;

		/**
		 * @brief Look an extension up by identifier.
		 * @param id Identifier the manager assigned at discovery.
		 * @return The extension, or nullptr if no extension has that identifier.
		 * @note The pointer points into the list of the manager and stays valid until the next
		 * Manager::Initialize rebuilds it.
		 */
		[[nodiscard]] const Extension* FindExtension(UniqueId id) const noexcept;

		/**
		 * @brief Get every extension the manager knows about.
		 * @return Pointers to all extensions, whatever state they are in.
		 */
		[[nodiscard]] std::vector<const Extension*> GetExtensions() const;

		// Service access helpers

		/**
		 * @brief Resolve a service from the container of the framework.
		 * @tparam Service The interface to resolve.
		 * @return The service.
		 * @note Throws if the service is not registered; use @ref TryResolve to test instead.
		 */
		template <typename Service>
		[[nodiscard]] std::shared_ptr<Service> Resolve() const {
			return GetServices().Resolve<Service>();
		}

		/**
		 * @brief Try to resolve a service from the container of the framework.
		 * @tparam Service The interface to resolve.
		 * @return The service, or nullptr if it is not registered.
		 */
		template <typename Service>
		[[nodiscard]] std::shared_ptr<Service> TryResolve() const noexcept {
			return GetServices().TryResolve<Service>();
		}

		/**
		 * @brief Compare two providers for equality.
		 * @param other The provider to compare against.
		 * @return True if both are views of the same framework instance.
		 */
		[[nodiscard]] bool operator==(const Provider& other) const noexcept;

		/**
		 * @brief Order two providers.
		 * @param other The provider to compare against.
		 * @return The relative ordering of the two providers.
		 */
		[[nodiscard]] auto operator<=>(const Provider& other) const noexcept;

	private:
		/**
		 * @brief Get the service container.
		 * @return The container every service was registered in.
		 */
		[[nodiscard]] const ServiceLocator& GetServices() const noexcept;

	private:
		struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
