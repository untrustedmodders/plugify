#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

#include <plugify/api/handle.hpp>
#include <plugify/api/config.hpp>
#include <plugify/api/version.hpp>

#include <plugify_export.h>

namespace plugify {
	class ILogger;
	class IAssemblyLoader;
	class ProviderHandle;
	class ManagerHandle;
	class Plugify;
	enum class Severity;

	/**
	 * @class PlugifyHandle
	 * @brief Wrapper handle for the Plugify, which is provided to the user and implemented in the core.
	 */
	class PLUGIFY_API PlugifyHandle : public Handle<const Plugify> {
		using Handle::Handle;
	public:
		/**
		 * @brief Initialize the Plugify system.
		 * @param rootDir The root directory for Plugify (optional).
		 * @return True if initialization is successful, false otherwise.
		 */
		bool Initialize(const std::filesystem::path& rootDir = {}) noexcept;

		/**
		 * @brief Terminate the Plugify system.
		 */
		void Terminate() noexcept;

		/**
		 * @brief Check if the Plugify system is initialized.
		 * @return True if initialized, false otherwise.
		 */
		bool IsInitialized() const noexcept;

		/**
		 * @brief Update the Plugify system.
		 * @noreturn
		 */
		void Update() noexcept;

		/**
		 * @brief Set the logger for the Plugify system.
		 * @param logger The logger to set.
		 * @noreturn
		 */
		void SetLogger(std::shared_ptr<ILogger> logger) const noexcept;

		/**
		 * @brief Log a message with the specified severity level.
		 * @param msg The log message.
		 * @param severity The severity level of the log message.
		 */
		void Log(std::string_view msg, Severity severity) const noexcept;

		/**
		 * @brief Get a weak pointer to the Provider.
		 * @return Weak pointer to the Provider.
		 */
		ProviderHandle GetProvider() const noexcept;

		/**
		 * @brief Get a weak pointer to the Manager.
		 * @return Weak pointer to the Manager.
		 */
		ManagerHandle GetManager() const noexcept;

		/**
		 * @brief Get the configuration of the Plugify system.
		 * @return Reference to the configuration.
		 */
		const Config& GetConfig() const noexcept;

		/**
		 * @brief Get the version information of the Plugify system.
		 * @return Version information.
		 */
		plg::version GetVersion() const noexcept;

		/**
		 * @brief Set the assembly loader for the Plugify system.
		 * @param loader The loader to set.
		 * @noreturn
		 */
		void SetAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader) noexcept;


		std::shared_ptr<IAssemblyLoader> GetAssemblyLoader() const noexcept;
	};

	/**
	 * @brief Factory function to create an instance of IPlugify.
	 * @return Shared pointer to the created IPlugify instance.
	 */
	PLUGIFY_API PlugifyHandle MakePlugify();
} // namespace plugify
