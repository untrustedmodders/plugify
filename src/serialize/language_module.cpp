#include "language_module.h"
#include <utils/file_system.h>
#include <wizard/language_module_descriptor.h>

namespace wizard {
    bool LoadLanguageModuleDescriptor(LanguageModuleDescriptor& desc, const fs::path& filePath) {
        auto json = FileSystem::ReadText(filePath);
        if (json.empty())
            return false;

        utils::json::Document doc;
        doc.Parse(json.c_str());

        if (doc.HasParseError()) {
            WZ_LOG_ERROR("File: '{}' could not be parsed correctly: {}", filePath.string(), utils::json::GetParseError_En(doc.GetParseError()));
            return false;
        }

        return ReadLanguageModuleDescriptor(desc, doc.GetObject());
    }

    bool ReadLanguageModuleDescriptor(LanguageModuleDescriptor& desc, const utils::json::Value& object) {
        if (!object.IsObject()) {
            WZ_LOG_ERROR("LanguageModuleDescriptor should be a valid object!");
            return false;
        }

        desc.supportedPlatforms.clear();

        for (auto itr = object.MemberBegin(); itr != object.MemberEnd(); ++itr) {
            const char* blockName = itr->name.GetString();
            const auto& value = itr->value;

            if (!strcmp(blockName, "fileVersion")) {
                if (value.IsInt()) {
                    desc.fileVersion = value.GetInt();
                }
            } else if (!strcmp(blockName, "version")) {
                if (value.IsInt()) {
                    desc.version = value.GetInt();
                }
            } else if (!strcmp(blockName, "versionName")) {
                if (value.IsString()) {
                    desc.versionName = value.GetString();
                }
            } else if (!strcmp(blockName, "friendlyName")) {
                if (value.IsString()) {
                    desc.friendlyName = value.GetString();
                }
            } else if (!strcmp(blockName, "language")) {
                if (value.IsString()) {
                    desc.language = value.GetString();
                }
            } else if (!strcmp(blockName, "description")) {
                if (value.IsString()) {
                    desc.description = value.GetString();
                }
            } else if (!strcmp(blockName, "createdBy")) {
                if (value.IsString()) {
                    desc.createdBy = value.GetString();
                }
            } else if (!strcmp(blockName, "createdByURL")) {
                if (value.IsString()) {
                    desc.createdByURL = value.GetString();
                }
            } else if (!strcmp(blockName, "docsURL")) {
                if (value.IsString()) {
                    desc.docsURL = value.GetString();
                }
            } else if (!strcmp(blockName, "downloadURL")) {
                if (value.IsString()) {
                    desc.downloadURL = value.GetString();
                }
            } else if (!strcmp(blockName, "supportURL")) {
                if (value.IsString()) {
                    desc.supportURL = value.GetString();
                }
            } else if (!strcmp(blockName, "supportedPlatforms")) {
                if (value.IsArray()) {
                    for (const auto& val : value.GetArray()) {
                        if (val.IsString()) {
                            desc.supportedPlatforms.emplace_back(val.GetString());
                        }
                    }
                }
            } else if (!strcmp(blockName, "forceLoad")) {
                if (value.IsBool()) {
                    desc.forceLoad = value.GetBool();
                }
            }
        }

        return true;
    }

    bool ReadLanguageModuleInfo(LanguageModuleInfo& info, const utils::json::Value& object) {
        if (!object.IsObject()) {
            WZ_LOG_ERROR("LanguageModuleInfo should be a valid object!");
            return false;
        }

        for (auto itr = object.MemberBegin(); itr != object.MemberEnd(); ++itr) {
            const char* blockName = itr->name.GetString();
            const auto& value = itr->value;

            if (!strcmp(blockName, "name")) {
                if (value.IsString()) {
                    info.name = value.GetString();
                }
            }
        }

        if (info.name.empty()) {
            WZ_LOG_ERROR("LanguageModuleInfo should contain valid name!");
            return false;
        }

        return true;
    }
}