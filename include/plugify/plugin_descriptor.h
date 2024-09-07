#pragma once

#include <span>
#include <string>
#include <plugify/method.h>
#include <plugify/reference_wrapper.h>
#include <plugify_export.h>

// TODO: Write comments
namespace plugify {
	struct PluginDescriptor;
	class PluginReferenceDescriptorRef;
	class MethodRef;

	class PLUGIFY_API PluginDescriptorRef : public Ref<const PluginDescriptor> {
		using Ref::Ref;
	public:
		[[nodiscard]] int32_t GetFileVersion() const noexcept;
		[[nodiscard]] int32_t GetVersion() const noexcept;
		[[nodiscard]] std::string_view GetVersionName() const noexcept;
		[[nodiscard]] std::string_view GetFriendlyName() const noexcept;
		[[nodiscard]] std::string_view GetDescription() const noexcept;
		[[nodiscard]] std::string_view GetCreatedBy() const noexcept;
		[[nodiscard]] std::string_view GetCreatedByURL() const noexcept;
		[[nodiscard]] std::string_view GetDocsURL() const noexcept;
		[[nodiscard]] std::string_view GetDownloadURL() const noexcept;
		[[nodiscard]] std::string_view GetUpdateURL() const noexcept;
		[[nodiscard]] std::span<std::string_view> GetSupportedPlatforms() const noexcept;
		[[nodiscard]] std::span<std::string_view> GetResourceDirectories() const noexcept;
		[[nodiscard]] std::string_view GetEntryPoint() const noexcept;
		[[nodiscard]] std::string_view GetLanguageModule() const noexcept;
		[[nodiscard]] std::span<const PluginReferenceDescriptorRef> GetDependencies() const noexcept;
		[[nodiscard]] std::span<const MethodRef> GetExportedMethods() const noexcept;
	};
	static_assert(is_ref_v<PluginDescriptorRef>);
}