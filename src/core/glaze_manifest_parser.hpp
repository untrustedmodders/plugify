#pragma once

#include "plugify/manifest_parser.hpp"

#include "core/glaze_metadata.hpp"

namespace plugify {
	/**
	 * @brief A manifest parser using the Glaze library
	 */
	class GlazeManifestParser : public IManifestParser {
	public:
		Result<Manifest>
		Parse(const std::string& content, [[maybe_unused]] ExtensionType type) override {
			auto parsed = glz::read_jsonc<Manifest>(content);
			if (!parsed) {
				return MakeError(glz::format_error(parsed.error(), content));
			}
			auto result = parsed->Validate();
			if (!result) {
				return MakeError(std::move(result.error()));
			}
			return *parsed;
		}
	};
}