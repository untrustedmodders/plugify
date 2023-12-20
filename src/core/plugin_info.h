#pragma once

namespace wizard {
    struct PluginInfo {
        std::string name;
        std::string description;
        std::string author;
        std::string version;
        std::string url;
        std::vector<std::string> dependencies;
    };
}