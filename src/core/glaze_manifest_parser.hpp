#pragma once

#include "plugify/manifest_parser.hpp"

#include "core/glaze_metadata.hpp"

namespace plugify {
	/**
	 * @brief A manifest parser using the Glaze library
	 */
	class GlazeManifestParser final : public IManifestParser {
	public:
		Result<Manifest> Parse(const std::string& content, [[maybe_unused]] ExtensionType type) override {
			auto parsed = glz::read_jsonc<Manifest>(content);
			if (!parsed) {
				return MakeError(glz::format_error(parsed.error(), content));
			}
			// Resolve() first: it links every by-name prototype/enum reference to
			// its definition, which Validate() then relies on being present.
			if (auto result = parsed->Resolve(); !result) {
				return MakeError(std::move(result.error()));
			}
			if (auto result = parsed->Validate(); !result) {
				return MakeError(std::move(result.error()));
			}
			return std::move(*parsed);
		}
	};
}
