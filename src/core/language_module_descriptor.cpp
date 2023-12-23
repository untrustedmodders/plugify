#include "language_module_descriptor.h"

#include <rapidjson/document.h>

using namespace wizard;

bool LanguageModuleInfo::Read(const rapidjson::Value& object) {
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
        WIZARD_LOG("LanguageModuleInfo: Should contain valid name!", ErrorLevel::ERROR);
        return false;
    }

    return true;
}
