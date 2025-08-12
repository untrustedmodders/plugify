#pragma once

#include <string>
#include <variant>
#include <vector>
#include <span>
#include <filesystem>
#include <unordered_map>

#include "package.hpp"
#include "package_manager.hpp"

namespace plugify {
	/*struct OperationResult {
		std::string message;
		std::vector<std::string> affectedPackages;
		std::optional<std::string> errorCode;
	};*/

	/**
	 * @brief Abstract repository interface for package sources
	 */
	class IRepository {
	public:
		virtual ~IRepository() = default;

		/**
		 * @brief Get repository name
		 */
		virtual std::string_view GetName() const = 0;

		/*
		 * @brief Get repository priority for sorting
		 */
		virtual int GetPriority() const = 0;

		/**
		 * @brief Check if repository is available/online
		 */
		virtual Result<bool> IsAvailable() = 0;

		/**
		 * @brief List all packages in repository
		 */
		virtual std::vector<Package> GetPackages() const = 0;
	};

	/**
	 * @brief Search criteria for querying packages
	 */
	/*struct PackageQuery {
		std::optional<std::string> namePattern;
		std::optional<PackageType> type;
		std::optional<PackageState> state;
		std::optional<std::string> author;
		std::optional<std::vector<std::string>> tags;
	};*/

} // namespace plugify
