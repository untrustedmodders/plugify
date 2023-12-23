#pragma once

namespace wizard {
    class WIZARD_API Paths {
    public:
        Paths() = delete;

        static const fs::path& ModulesDir();
        static const fs::path& PluginsDir();
    };
}