#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <plugify/reference_wrapper.h>
#include <plugify_export.h>

// TODO: Write comments
namespace plugify {
	struct LanguageModuleDescriptor;

	class PLUGIFY_API LanguageModuleDescriptorRef : public Ref<const LanguageModuleDescriptor> {
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
		[[nodiscard]] std::span<const std::string> GetLibraryDirectories() const noexcept;
		[[nodiscard]] const std::string& GetLanguage() const noexcept;
		[[nodiscard]] bool IsForceLoad() const noexcept;
	};
	static_assert(is_ref_v<LanguageModuleDescriptorRef>);
}