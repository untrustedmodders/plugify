#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <plugify/reference_wrapper.h>
#include <plugify_export.h>

// TODO: Write comments
namespace plugify {
#if PLUGIFY_CORE
	struct LanguageModuleDescriptor;
#endif

	class PLUGIFY_API LanguageModuleDescriptorRef {
		PLUGUFY_REFERENCE(LanguageModuleDescriptorRef, const LanguageModuleDescriptor)
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
		[[nodiscard]] std::span<const std::string> GetSupportedPlatforms() const;
		[[nodiscard]] std::span<const std::string> GetResourceDirectories() const;
		[[nodiscard]] std::span<const std::string> GetLibraryDirectories() const;
		[[nodiscard]] std::string_view GetLanguage() const noexcept;
		[[nodiscard]] bool IsForceLoad() const noexcept;
	};
	static_assert(is_ref_v<LanguageModuleDescriptorRef>);
}