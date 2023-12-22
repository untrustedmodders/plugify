#include "paths.h"

const fs::path& wizard::Paths::ModulesDir() {
    static fs::path modulesDir{"docs/modules/" };
    return modulesDir;
}

const fs::path& wizard::Paths::PluginsDir() {
    static fs::path modulesDir{"docs/plugins/" };
    return modulesDir;
}

std::string wizard::Paths::GetExtension(const fs::path& filepath) {
    return String::Lowercase(filepath.extension().string());
}
