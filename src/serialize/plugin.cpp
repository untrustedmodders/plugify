#include "plugin.h"
#include "method.h"
#include "language_module.h"
#include <utils/file_system.h>
#include <wizard/plugin_descriptor.h>

namespace wizard {
    bool LoadPluginDescriptor(PluginDescriptor& desc, const fs::path& filePath) {
        auto json = FileSystem::ReadText(filePath);
        if (json.empty())
            return false;

        utils::json::Document doc;
        doc.Parse(json.c_str());

        if (doc.HasParseError()) {
            WIZARD_LOG("File: '" + filePath.string() + "' could not be parsed correctly: " + utils::json::GetParseError_En(doc.GetParseError()), ErrorLevel::ERROR);
            return false;
        }

        return ReadPluginDescriptor(desc, doc.GetObject());
    }

    bool ReadPluginDescriptor(PluginDescriptor& desc, const utils::json::Value& object) {
        if (!object.IsObject()) {
            WIZARD_LOG("PluginDescriptor should be a valid object!", ErrorLevel::ERROR);
            return false;
        }

        desc.supportedPlatforms.clear();
        desc.dependencies.clear();
        desc.exportedMethods.clear();

        for (auto itr = object.MemberBegin(); itr != object.MemberEnd(); ++itr) {
            const char* blockName = itr->name.GetString();
            const auto& value = itr->value;

            if (!strcmp(blockName, "fileVersion")) {
                if (value.IsInt()) {
                    desc.fileVersion = value.GetInt();
                }
            }
            else if(!strcmp(blockName, "version")) {
                if (value.IsInt()) {
                    desc.version = value.GetInt();
                }
            }
            else if(!strcmp(blockName, "versionName")) {
                if (value.IsString()) {
                    desc.versionName = value.GetString();
                }
            }
            else if(!strcmp(blockName, "friendlyName")) {
                if (value.IsString()) {
                    desc.friendlyName = value.GetString();
                }
            }
            else if(!strcmp(blockName, "description")) {
                if (value.IsString()) {
                    desc.description = value.GetString();
                }
            }
            else if(!strcmp(blockName, "createdBy")) {
                if (value.IsString()) {
                    desc.createdBy = value.GetString();
                }
            }
            else if(!strcmp(blockName, "createdByURL")) {
                if (value.IsString()) {
                    desc.createdByURL = value.GetString();
                }
            }
            else if(!strcmp(blockName, "docsURL")) {
                if (value.IsString()) {
                    desc.docsURL = value.GetString();
                }
            }
            else if(!strcmp(blockName, "downloadURL")) {
                if (value.IsString()) {
                    desc.downloadURL = value.GetString();
                }
            }
            else if(!strcmp(blockName, "supportURL")) {
                if (value.IsString()) {
                    desc.supportURL = value.GetString();
                }
            }
            else if(!strcmp(blockName, "assemblyPath")) {
                if (value.IsString()) {
                    desc.assemblyPath = value.GetString();
                }
            }
            else if(!strcmp(blockName, "supportedPlatforms")) {
                if (value.IsArray()) {
                    for (const auto& val : value.GetArray()) {
                        if (val.IsString()) {
                            desc.supportedPlatforms.emplace_back(val.GetString());
                        }
                    }
                }
            }
            else if(!strcmp(blockName, "languageModule")) {
                if (value.IsObject()) {
                    ReadLanguageModuleInfo(desc.languageModule, value);
                }
            }
            else if(!strcmp(blockName, "dependencies")) {
                if (value.IsArray()) {
                    for (const auto& val : value.GetArray()) {
                        if (val.IsObject()) {
                            ReadPluginReferenceDescriptor(desc.dependencies.emplace_back(), val);
                        }
                    }
                }
            }
            else if(!strcmp(blockName, "exportedMethods")) {
                if (value.IsArray()) {
                    for (const auto& val : value.GetArray()) {
                        if (val.IsObject()) {
                            ReadMethod(desc.exportedMethods.emplace_back(), val);
                        }
                    }
                }
            }
        }

        return true;
    }

    bool ReadPluginReferenceDescriptor(PluginReferenceDescriptor& desc, const utils::json::Value& object) {
        if (!object.IsObject()) {
            WIZARD_LOG("PluginReferenceDescriptor should be a valid object!", ErrorLevel::ERROR);
            return false;
        }

        desc.supportedPlatforms.clear();

        for (auto itr = object.MemberBegin(); itr != object.MemberEnd(); ++itr) {
            const char* blockName = itr->name.GetString();
            const auto& value = itr->value;

            if (!strcmp(blockName, "name")) {
                if (value.IsString()) {
                    desc.name = value.GetString();
                }
            }
            else if(!strcmp(blockName, "optional")) {
                if (value.IsBool()) {
                    desc.optional = value.GetBool();
                }
            }
            else if(!strcmp(blockName, "description")) {
                if (value.IsString()) {
                    desc.description = value.GetString();
                }
            }
            else if(!strcmp(blockName, "downloadURL")) {
                if (value.IsString()) {
                    desc.downloadURL = value.GetString();
                }
            }
            else if(!strcmp(blockName, "supportedPlatforms")) {
                if (value.IsArray()) {
                    for (const auto& val : value.GetArray()) {
                        if (val.IsString()) {
                            desc.supportedPlatforms.emplace_back(val.GetString());
                        }
                    }
                }
            }
            else if(!strcmp(blockName, "requestedVersion")) {
                if (value.IsInt()) {
                    desc.requestedVersion = std::make_optional<int32_t>(value.GetInt());
                }
            }
        }

        if (desc.name.empty()) {
            WIZARD_LOG("PluginReferenceDescriptor: Should contain valid name!", ErrorLevel::ERROR);
            return false;
        }

        return true;
    }
}