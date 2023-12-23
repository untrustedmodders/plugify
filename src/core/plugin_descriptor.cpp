#include "plugin_descriptor.h"
#include "utils/file_system.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

using namespace wizard;

bool PluginDescriptor::IsSupportsPlatform(const std::string& platform) const {
    return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), platform) != supportedPlatforms.end();
}

bool PluginDescriptor::Load(const fs::path& filePath) {
    auto json = FileSystem::ReadText(filePath);
    if (json.empty())
        return false;

    rapidjson::Document doc;
    doc.Parse(json.c_str());

    if (doc.HasParseError()) {
        WIZARD_LOG("File: '" + filePath.string() + "' could not be parsed correctly: " + rapidjson::GetParseError_En(doc.GetParseError()), ErrorLevel::ERROR);
        return false;
    }

    return Read(doc.GetObject());
}

bool PluginDescriptor::Read(const rapidjson::Value& object) {
    if (!object.IsObject()) {
        WIZARD_LOG("PluginDescriptor should be a valid object!", ErrorLevel::ERROR);
        return false;
    }
    
    supportedPlatforms.clear();
    dependencies.clear();

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
        else if(!strcmp(blockName, "languageModule")) {
            if (value.IsObject()) {
                languageModule.Read(value);
            }
        }
        else if(!strcmp(blockName, "dependencies")) {
            if (value.IsArray()) {
                for (const auto& val : value.GetArray()) {
                    if (val.IsObject()) {
                        dependencies.emplace_back().Read(val);
                    }
                }
            }
        }
    }

    return true;
}
