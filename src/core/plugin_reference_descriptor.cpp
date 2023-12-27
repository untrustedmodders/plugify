#include <wizard/plugin_reference_descriptor.h>
#include <wizard/plugin_reference_descriptor.h>

using namespace wizard;

PluginReferenceDescriptor::PluginReferenceDescriptor(std::string pluginName) : name{std::move(pluginName)} {
}

bool PluginReferenceDescriptor::IsSupportsPlatform(const std::string& platform) const {
    return supportedPlatforms.empty() || std::find(supportedPlatforms.begin(), supportedPlatforms.end(), platform) != supportedPlatforms.end();
}

bool PluginReferenceDescriptor::Read(const utils::json::Value& object) {
    supportedPlatforms.clear();

    for (auto itr = object.MemberBegin(); itr != object.MemberEnd(); ++itr) {
        const char* blockName = itr->name.GetString();
        const auto& value = itr->value;

        if (!strcmp(blockName, "name")) {
            if (value.IsString()) {
                name = value.GetString();
            }
        }
        else if(!strcmp(blockName, "optional")) {
            if (value.IsBool()) {
                optional = value.GetBool();
            }
        }
        else if(!strcmp(blockName, "description")) {
            if (value.IsString()) {
                description = value.GetString();
            }
        }
        else if(!strcmp(blockName, "downloadURL")) {
            if (value.IsString()) {
                downloadURL = value.GetString();
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
        else if(!strcmp(blockName, "requestedVersion")) {
            if (value.IsInt()) {
                requestedVersion = std::make_optional<int32_t>(value.GetInt());
            }
        }
    }

    if (name.empty()) {
        WIZARD_LOG("PluginReferenceDescriptor Should contain valid name!", ErrorLevel::ERROR);
        return false;
    }

    return true;
}
