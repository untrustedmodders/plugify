#include "plugin_glue.h"
#include "plugin_manager.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

using namespace wizard;

#define WIZARD_ADD_INTERNAL_CALL(name) mono_add_internal_call("Wizard.InternalCalls::" #name, (const void*)&(name))

static MonoObject* Plugin_FindPluginByName(MonoString* name) {
    char* nameCStr = mono_string_to_utf8(name);
    auto plugin = PluginManager::Get().findPlugin(nameCStr);
    mono_free(nameCStr);
    return plugin != nullptr ? plugin->getManagedObject() : nullptr;
}

void PluginGlue::RegisterFunctions() {
    WIZARD_ADD_INTERNAL_CALL(Plugin_FindPluginByName);
}