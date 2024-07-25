#pragma once

#include <span>
#include <string>
#include <optional>
#include <plugify/reference_wrapper.h>
#include <plugify_export.h>

// TODO: Write comments
namespace plugify {
	struct PluginReferenceDescriptor;

	class PLUGIFY_API PluginReferenceDescriptorRef : public Ref<const PluginReferenceDescriptor> {
		using Ref::Ref;
	public:
		std::string_view GetName() const noexcept;
		bool IsOptional() const noexcept;
		std::span<const std::string> GetSupportedPlatforms() const noexcept;
		std::optional<int32_t> GetRequestedVersion() const noexcept;
	};
	static_assert(is_ref_v<PluginReferenceDescriptorRef>);
}