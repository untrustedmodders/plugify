#pragma once

#include <plg/expected.hpp>
#include <plg/string.hpp>
#include <plugify/api/path.hpp>

namespace plugify {
	/**
	 * @typedef AssemblyResult
	 * @brief Result type for assembly loading operations.
	 *
	 * Represents either a successfully loaded assembly or an error.
	 */
	using Readesult = plg::expected<plg::string, plg::string>;

	/**
	 * @brief File system interface
	 */
	class IFileSystem {
	public:
		virtual ~IFileSystem() = default;

		/**
		 * @brief Parse manifest from file
		 * @
		 */
		virtual Readesult ReadFile(std::filesystem::path_view path) = 0;
	};

}