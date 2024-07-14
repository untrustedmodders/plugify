#pragma once

#include <vector>
#include <optional>
#include <string_view>
#include <plugify/reference_wrapper.h>
#include <plugify_export.h>

// TODO: Write comments
namespace plugify {
#if PLUGIFY_CORE
	struct PluginReferenceDescriptor;
#endif

	class PLUGIFY_API PluginReferenceDescriptorRef {
		PLUGUFY_REFERENCE(PluginReferenceDescriptorRef, const PluginReferenceDescriptor)
	public:
		std::string_view GetName() const noexcept;
		bool IsOptional() const noexcept;
		std::vector<std::string_view> GetSupportedPlatforms() const;
		std::optional<int32_t> GetRequestedVersion() const noexcept;
	};
	static_assert(is_ref_v<PluginReferenceDescriptorRef>);
}