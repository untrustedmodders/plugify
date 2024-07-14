#pragma once

namespace plugify {
	struct PluginReferenceDescriptor final {
		std::string name;
		bool optional{false};
		//std::string description;
		//std::string downloadURL;
		std::vector<std::string> supportedPlatforms;
		std::optional<int32_t> requestedVersion;

		[[nodiscard]] bool operator==(const PluginReferenceDescriptor& rhs) const noexcept { return name == rhs.name; }
	};
}