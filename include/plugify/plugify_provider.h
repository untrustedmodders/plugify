#pragma once

#include <plugify_export.h>
#include <filesystem>
#include <string>
#include <memory>

namespace plugify {
	class PlugifyProvider;
	enum class Severity : uint8_t;

	/**
 	 * @brief Interface class for the PlugifyProvider, which is provided to the user and implemented in the core.
	 *        The PlugifyProvider is responsible for managing and providing essential functionality to the Plugify system.
	 */
	class PLUGIFY_API IPlugifyProvider {
	protected:
		explicit IPlugifyProvider(PlugifyProvider& impl);
		~IPlugifyProvider();

	public:
		/**
		 * @brief Log a message with a specified severity level.
		 * @param msg The message to log.
		 * @param severity The severity level of the log message.
		 */
		void Log(const std::string& msg, Severity severity) const;

		/**
		 * @brief Get the base directory of the Plugify system.
		 * @return Reference to the base directory path.
		 */
		const std::filesystem::path& GetBaseDir() const;

	private:
		PlugifyProvider& _impl; ///< Reference to the underlying PlugifyProvider implementation.
	};
} // namespace plugify
