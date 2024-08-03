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
		[[nodiscard]] const std::string& GetVersionName() const noexcept;
		[[nodiscard]] const std::string& GetFriendlyName() const noexcept;
		[[nodiscard]] const std::string& GetDescription() const noexcept;
		[[nodiscard]] const std::string& GetCreatedBy() const noexcept;
		[[nodiscard]] const std::string& GetCreatedByURL() const noexcept;
		[[nodiscard]] const std::string& GetDocsURL() const noexcept;
		[[nodiscard]] const std::string& GetDownloadURL() const noexcept;
		[[nodiscard]] const std::string& GetUpdateURL() const noexcept;
		[[nodiscard]] std::span<const std::string> GetSupportedPlatforms() const noexcept;
		[[nodiscard]] std::span<const std::string> GetResourceDirectories() const noexcept;
		[[nodiscard]] const std::string& GetEntryPoint() const noexcept;
		[[nodiscard]] const std::string& GetLanguageModule() const noexcept;
		[[nodiscard]] std::span<const PluginReferenceDescriptorRef> GetDependencies() const noexcept;
		[[nodiscard]] std::span<const MethodRef> GetExportedMethods() const noexcept;
	};
	static_assert(is_ref_v<PluginDescriptorRef>);
}