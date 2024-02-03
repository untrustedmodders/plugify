#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace plugify {
	struct PluginReferenceDescriptor {
		std::string name;
		bool optional{ false };
		//std::string description;
		//std::string downloadURL;
		std::vector<std::string> supportedPlatforms;
		std::optional<std::int32_t> requestedVersion;

		bool operator==(const PluginReferenceDescriptor& rhs) const { return name == rhs.name; }
	};
}