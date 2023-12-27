#pragma once

namespace wizard {
    struct LanguageModuleDescriptor;
    struct LanguageModuleInfo;

    bool LoadLanguageModuleDescriptor(LanguageModuleDescriptor& desc, const fs::path& filePath);
    bool ReadLanguageModuleDescriptor(LanguageModuleDescriptor& desc, const utils::json::Value& object);

    bool ReadLanguageModuleInfo(LanguageModuleInfo& info, const utils::json::Value& object);
}