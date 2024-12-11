#pragma once

namespace plugify {
	struct PluginReferenceDescriptor final {
		std::string name;
		bool optional{false};
		//std::string description;
		//std::string downloadURL;
		std::vector<std::string> supportedPlatforms;
		std::optional<int32_t> requestedVersion;

		bool operator==(const PluginReferenceDescriptor& rhs) const noexcept { return name == rhs.name; }

	private:
		mutable std::shared_ptr<std::vector<std::string_view>> _supportedPlatforms;
		friend class PluginReferenceDescriptorRef;
	};
}
