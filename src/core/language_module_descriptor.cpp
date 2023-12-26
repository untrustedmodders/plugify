#include <wizard/language_module_descriptor.h>
#include "utils/file_system.h"

using namespace wizard;

bool LanguageModuleDescriptor::IsSupportsPlatform(const std::string& platform) const {
    return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), platform) != supportedPlatforms.end();
}

bool LanguageModuleDescriptor::Load(const fs::path& filePath) {
    auto json = FileSystem::ReadText(filePath);
    if (json.empty())
        return false;

    utils::json::Document doc;
    doc.Parse(json.c_str());

    if (doc.HasParseError()) {
        WIZARD_LOG("File: '" + filePath.string() + "' could not be parsed correctly: " + utils::json::GetParseError_En(doc.GetParseError()), ErrorLevel::ERROR);
        return false;
    }

    return Read(doc.GetObject());
}

bool LanguageModuleDescriptor::Read(const utils::json::Value& object) {
    if (!object.IsObject()) {
        WIZARD_LOG("LanguageModuleDescriptor should be a valid object!", ErrorLevel::ERROR);
        return false;
    }

    supportedPlatforms.clear();

    for (auto itr = object.MemberBegin(); itr != object.MemberEnd(); ++itr) {
        const char* blockName = itr->name.GetString();
        const auto& value = itr->value;

        if (!strcmp(blockName, "fileVersion")) {
            if (value.IsInt()) {
                fileVersion = value.GetInt();
            }
        }
        else if(!strcmp(blockName, "version")) {
            if (value.IsInt()) {
                version = value.GetInt();
            }
        }
        else if(!strcmp(blockName, "versionName")) {
            if (value.IsString()) {
                versionName = value.GetString();
            }
        }
        else if(!strcmp(blockName, "friendlyName")) {
            if (value.IsString()) {
                friendlyName = value.GetString();
            }
        }
        else if(!strcmp(blockName, "language")) {
            if (value.IsString()) {
                language = value.GetString();
            }
        }
        else if(!strcmp(blockName, "description")) {
            if (value.IsString()) {
                description = value.GetString();
            }
        }
        else if(!strcmp(blockName, "createdBy")) {
            if (value.IsString()) {
                createdBy = value.GetString();
            }
        }
        else if(!strcmp(blockName, "createdByURL")) {
            if (value.IsString()) {
                createdByURL = value.GetString();
            }
        }
        else if(!strcmp(blockName, "docsURL")) {
            if (value.IsString()) {
                docsURL = value.GetString();
            }
        }
        else if(!strcmp(blockName, "downloadURL")) {
            if (value.IsString()) {
                downloadURL = value.GetString();
            }
        }
        else if(!strcmp(blockName, "supportURL")) {
            if (value.IsString()) {
                supportURL = value.GetString();
            }
        }
        else if(!strcmp(blockName, "supportedPlatforms")) {
            if (value.IsArray()) {
                for (const auto& val : value.GetArray()) {
                    if (val.IsString()) {
                        supportedPlatforms.emplace_back(val.GetString());
                    }
                }
            }
        }
        else if(!strcmp(blockName, "forceLoad")) {
            if (value.IsBool()) {
                forceLoad = value.GetBool();
            }
        }
    }

    return true;
}

bool LanguageModuleInfo::Read(const utils::json::Value& object) {
    for (auto itr = object.MemberBegin(); itr != object.MemberEnd(); ++itr) {
        const char* blockName = itr->name.GetString();
        const auto& value = itr->value;

        if (!strcmp(blockName, "name")) {
            if (value.IsString()) {
                name = value.GetString();
            }
        }
    }

    if (name.empty()) {
        WIZARD_LOG("LanguageModuleInfo should contain valid name!", ErrorLevel::ERROR);
        return false;
    }

    return true;
}