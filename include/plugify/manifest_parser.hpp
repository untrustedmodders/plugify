#pragma once

#include <filesystem>
#include <string>

#include "plugify/manifest.hpp"
#include "plugify/types.hpp"

namespace plugify {
	/**
	 * @brief Interface for parsing manifest files
	 */
	class IManifestParser {
	public:
		virtual ~IManifestParser() = default;

		/**
		 * @brief Parse the manifest content from a string
		 * @param content The content of the manifest file as a string
		 * @param type The manifest type
		 * @return A Result containing the parsed Manifest or an error
		 */
		virtual Result<Manifest> Parse(const std::string& content, ExtensionType type) = 0;
	};
}