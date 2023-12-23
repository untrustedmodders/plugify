#include "plugin_descriptor.h"

using namespace wizard;


int main() {
    LogSystem::SetLogger(std::make_shared<wizard::StdLogger>());

    WIZARD_LOG("Git: [" WIZARD_GIT_COMMIT_HASH "]:(" WIZARD_GIT_TAG ") - " WIZARD_GIT_COMMIT_SUBJECT " on " WIZARD_GIT_BRANCH " at '" WIZARD_GIT_COMMIT_DATE "'", ErrorLevel::INFO);
    WIZARD_LOG("Compiled on: " WIZARD_COMPILED_SYSTEM " from: " WIZARD_COMPILED_GENERATOR" with: '" WIZARD_COMPILED_COMPILER "'", ErrorLevel::INFO);

    PluginDescriptor descriptor;
    descriptor.Load("docs/plugins/Plugin1/Plugin1.wplugin");
}