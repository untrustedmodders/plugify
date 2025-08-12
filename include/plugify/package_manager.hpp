#pragma once

#include <optional>
#include <functional>
#include <string_view>
#include <filesystem>
#include <unordered_map>
#include <vector>

#include "resolver.hpp"
#include "repository.hpp"

namespace plugify {
	/**
	 * @brief Operation progress callback
	 */
	using ProgressCallback = std::function<void(std::string_view operation, float progress)>;

	/**
	 * @brief Operation result with detailed information
	 */
	/*struct OperationResult {
		std::string message;
		std::vector<std::string> warnings;
		std::vector<Package> affectedPackages;
	};*/

	/**
	 * @brief Main package manager interface
	 */
	class IPackageManager {
	public:
	    virtual ~IPackageManager() = default;


	};

} // namespace plugify
