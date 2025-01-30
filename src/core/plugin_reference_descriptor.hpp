#pragma once

namespace plugify {
	struct PluginReferenceDescriptor {
		std::string name;
		std::optional<bool> optional;
		//std::string description;
		//std::string downloadURL;
		std::optional<std::vector<std::string>> supportedPlatforms;
		std::optional<int32_t> requestedVersion;

		bool operator==(const PluginReferenceDescriptor& rhs) const noexcept { return name == rhs.name; }

	private:
		mutable std::shared_ptr<std::vector<std::string_view>> _supportedPlatforms;
		friend class PluginReferenceDescriptorHandle;
	};
}
