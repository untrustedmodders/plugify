#pragma once

namespace wizard {
    struct PluginDescriptor;
    struct PluginReferenceDescriptor;
    struct LanguageModuleInfo;

    bool LoadPluginDescriptor(PluginDescriptor& desc, const fs::path& filePath);
    bool ReadPluginDescriptor(PluginDescriptor& desc, const utils::json::Value& object);

    bool ReadPluginReferenceDescriptor(PluginReferenceDescriptor& desc, const utils::json::Value& object);
}