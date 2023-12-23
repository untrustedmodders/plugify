#pragma once

namespace wizard {
    struct LanguageModuleDescriptor {
        std::string name;

        // TODO: Add more
    };

    struct LanguageModuleInfo {
        std::string name;

        bool Read(const rapidjson::Value& object);
    };
}