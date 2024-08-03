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
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] bool IsOptional() const noexcept;
		[[nodiscard]] std::span<const std::string> GetSupportedPlatforms() const noexcept;
		[[nodiscard]] std::optional<int32_t> GetRequestedVersion() const noexcept;
	};
	static_assert(is_ref_v<PluginReferenceDescriptorRef>);
}