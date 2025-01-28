#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <plugify/config.hpp>
#include <plugify/version.hpp>
#include <plugify_export.h>
#include <span>

namespace plugify {
	class ILogger;
	class IPlugifyProvider;
	class IPluginManager;
	class IPackageManager;
	enum class Severity;

	/**
	 * @class IPlugify
	 * @brief Interface for the Plugify system.
	 *
	 * The IPlugify interface provides methods to initialize and terminate the Plugify system,
	 * set a logger, get various components, and retrieve configuration information.
	 */
	class IPlugify {
	public:
		virtual ~IPlugify() = default;

		/**
		 * @brief Initialize the Plugify system.
		 * @param rootDir The root directory for Plugify (optional).
		 * @return True if initialization is successful, false otherwise.
		 */
		virtual bool Initialize(const std::filesystem::path& rootDir = {}) = 0;

		/**
		 * @brief Terminate the Plugify system.
		 */
		virtual void Terminate() = 0;

		/**
		 * @brief Check if the Plugify system is initialized.
		 * @return True if initialized, false otherwise.
		 */
		virtual bool IsInitialized() const = 0;

		/**
		 * @brief Set the logger for the Plugify system.
		 * @param logger The logger to set.
		 * @noreturn
		 */
		virtual void SetLogger(std::shared_ptr<ILogger> logger) = 0;

		/**
		 * @brief Log a message with the specified severity level.
		 * @param msg The log message.
		 * @param severity The severity level of the log message.
		 */
		virtual void Log(std::string_view msg, Severity severity) = 0;
		
		/**
		 * @brief Add a repository to the config.
		 * 
		 * This method adds a new repository to the config, allowing it to search for
		 * packages in the specified repository when performing package-related operations.
		 * 
		 * @param repository The URL or path of the repository to add.
		 * @return True if the repository was successfully added, false otherwise.
		 */
		virtual bool AddRepository(std::string_view repository) = 0;

		/**
		 * @brief Get a weak pointer to the Plugify provider.
		 * @return Weak pointer to the Plugify provider.
		 */
		virtual std::weak_ptr<IPlugifyProvider> GetProvider() const = 0;

		/**
		 * @brief Get a weak pointer to the Plugin Manager.
		 * @return Weak pointer to the Plugin Manager.
		 */
		virtual std::weak_ptr<IPluginManager> GetPluginManager() const = 0;

		/**
		 * @brief Get a weak pointer to the Package Manager.
		 * @return Weak pointer to the Package Manager.
		 */
		virtual std::weak_ptr<IPackageManager> GetPackageManager() const = 0;

		/**
		 * @brief Get the configuration of the Plugify system.
		 * @return Reference to the configuration.
		 */
		virtual const Config& GetConfig() const = 0;

		/**
		 * @brief Get the version information of the Plugify system.
		 * @return Version information.
		 */
		virtual Version GetVersion() const = 0;
	};

	/**
	 * @brief Factory function to create an instance of IPlugify.
	 * @return Shared pointer to the created IPlugify instance.
	 */
	PLUGIFY_API std::shared_ptr<IPlugify> MakePlugify();
} // namespace plugify
