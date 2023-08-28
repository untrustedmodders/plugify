#include "plugin_manager.h"
#include "file_system.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

using namespace std::string_literals;

using namespace wizard;

namespace wizard::Utils {
    MonoAssembly* LoadMonoAssembly(const fs::path& assemblyPath, bool loadPDB) {
        MonoImageOpenStatus status = MONO_IMAGE_IMAGE_INVALID;
        MonoImage* image = nullptr;

        FileSystem::ReadBytes<char>(assemblyPath, [&image, &status](std::span<char> buffer) {
            image = mono_image_open_from_data_full(buffer.data(), static_cast<uint32_t>(buffer.size()), 1, &status, 0);
        });

        if (status != MONO_IMAGE_OK) {
            printf("Failed to load assembly file: %s", mono_image_strerror(status));
            return nullptr;
        }

        if (loadPDB) {
            fs::path pdbPath{ assemblyPath };
            pdbPath.replace_extension(".pdb");

            FileSystem::ReadBytes<mono_byte>(pdbPath, [&image, &pdbPath](std::span<mono_byte> buffer) {
                mono_debug_open_image_from_memory(image, buffer.data(), static_cast<int>(buffer.size()));
                printf("Loaded PDB: %s", pdbPath.string().c_str());
            });
        }

        MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.string().c_str(), &status, 0);
        mono_image_close(image);
        return assembly;
    }

    void PrintAssemblyTypes(MonoAssembly* assembly) {
        MonoImage* image = mono_assembly_get_image(assembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int numTypes = mono_table_info_get_rows(typeDefinitionsTable);

        for (int i = 0; i < numTypes; ++i) {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

            printf("%s.%s", nameSpace, name);
        }
    }

    std::string MonoStringToString(MonoString* string) {
        char* cStr = mono_string_to_utf8(string);
        std::string str{cStr};
        mono_free(cStr);
        return str;
    }

    const char* GetStatusText(PluginStatus status)  {
        switch (status) {
            case PluginStatus::NotLoaded:
                return "NOLOAD";
            case PluginStatus::Error:
                return "ERROR";
            case PluginStatus::Refused:
                return "FAILED";
            case PluginStatus::Paused:
                return "PAUSED";
            case PluginStatus::Running:
                return "RUNNING";
            default:
                return "-";
        }
    }
}

#define NOTIFY_OTHERS(method, id) \
    void* param = &id; \
    for (auto& [i, p] : plugins) { \
        if (i != id) { \
            p.invoke(p.method, &param); \
        } \
    } \

PluginManager::PluginManager() {
    initMono();

    //PluginGlue::RegisterFunctions();
}

PluginManager::~PluginManager() {
    shutdownMono();
}

PluginManager& PluginManager::Get() {
    static PluginManager Manager;
    return Manager;
}

void PluginManager::initMono() {
    mono_set_assemblies_path("mono/lib");

    if (enableDebugging) {
        const char* argv[2] = {
            "--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
            "--soft-breakpoints"
        };

        mono_jit_parse_options(2, (char**)argv);
        mono_debug_init(MONO_DEBUG_FORMAT_MONO);
    }

    rootDomain = mono_jit_init("WizardJITRuntime");
    assert(rootDomain);

    if (enableDebugging)
        mono_debug_domain_create(rootDomain);

    mono_thread_set_main(mono_thread_current());
}

void PluginManager::shutdownMono() {
    mono_domain_set(mono_get_root_domain(), false);

    if (appDomain) {
        mono_domain_unload(appDomain);
        appDomain = nullptr;
    }

    if (rootDomain) {
        mono_jit_cleanup(rootDomain);
        rootDomain = nullptr;
    }
}

void PluginManager::loadAll() {
    if (appDomain)
        return;

    // Create an app domain
    char appName[] = "WizardPluginRuntime";
    appDomain = mono_domain_create_appdomain(appName, nullptr);
    mono_domain_set(appDomain, true);

    // Load a core assembly
    if (!coreAssembly.load(corePath, enableDebugging))
        throw std::runtime_error("Core assembly '" + corePath.string() + "' is missing!");

    // Retrieve and instantiate core classes
    coreClasses.try_emplace("Plugin", coreAssembly, "Wizard", "Plugin");
    coreClasses.try_emplace("IPluginListener", coreAssembly, "Wizard", "IPluginListener");
    coreClasses.try_emplace("IPluginInfo", coreAssembly, "Wizard", "IPluginInfo");

    // Load a plugin assemblies
    for (auto const& entry : fs::directory_iterator(pluginsPath)) {
        const auto& path = entry.path();
        if (fs::is_regular_file(path) && path.extension().string() == ".dll") {
            PluginAssembly assembly;
            if (assembly.load(path, enableDebugging)) {
                assembly.loadClasses();

                for (const auto& [name, pluginClass] : assembly.getPluginClasses()) {
                    uint64_t pluginId = lastPluginId++;
                    plugins.try_emplace(pluginId, name, path, pluginClass, pluginId);
                }

                pluginAssemblies.try_emplace(path, std::move(assembly));
            }
        }
    }

    std::vector<PluginId> loaded;
    loaded.reserve(plugins.size());

    // Load a plugins
    for (auto& [id, plugin] : plugins) {
        bool ret = plugin.invokeOnLoad();
        if (ret) {
            loaded.push_back(id);
        }
    }

    for (auto id : loaded) {
        NOTIFY_OTHERS(onLoaded, id);
    }

    for (auto& [id, plugin] : plugins) {
        plugin.invoke(plugin.onAllLoaded);
    }
}

std::vector<PluginId> load(const fs::path& file, bool& already) {
    return {};
}

std::vector<PluginId> unload(const fs::path& file, bool force) {
    return {};
}

bool PluginManager::pause(PluginId id) {
    auto plugin = findPlugin(id);
    if (!plugin)
        return false;

    bool ret = plugin->invokeOnPause();
    if (ret) {
        NOTIFY_OTHERS(onPaused, plugin->id);
    }
    return ret;
}

bool PluginManager::unpause(PluginId id) {
    auto plugin = findPlugin(id);
    if (!plugin)
        return false;

    bool ret = plugin->invokeOnUnpause();
    if (ret) {
        NOTIFY_OTHERS(onUnpaused, plugin->id);
    }
    return ret;
}

PluginClass* PluginManager::findCoreClass(const std::string& name) {
    auto it = coreClasses.find(name);
    return it != coreClasses.end() ? &it->second : nullptr;
}

PluginInstance* PluginManager::findPlugin(PluginId id) {
    auto it = plugins.find(id);
    return it != plugins.end() ? &it->second : nullptr;
}

MonoString* PluginManager::createString(const char* string) const {
    return mono_string_new(appDomain, string);
}

MonoObject* PluginManager::instantiateClass(MonoClass* monoClass) const {
    MonoObject* instance = mono_object_new(appDomain, monoClass);
    mono_runtime_object_init(instance);
    return instance;
}
/*_________________________________________________*/

PluginAssembly::~PluginAssembly() {
    unload();
}

PluginAssembly::PluginAssembly(PluginAssembly&& other) noexcept {
    assembly = other.assembly;
    image = other.image;
    pluginClasses = std::move(other.pluginClasses);
    other.assembly = nullptr;
    other.image = nullptr;
}

PluginAssembly& PluginAssembly::operator=(PluginAssembly&& other) noexcept {
    assembly = other.assembly;
    image = other.image;
    pluginClasses = std::move(other.pluginClasses);
    other.assembly = nullptr;
    other.image = nullptr;
    return *this;
}

bool PluginAssembly::load(const fs::path& file, bool enableDebugging) {
    unload();

    assembly = Utils::LoadMonoAssembly(file, enableDebugging);
    if (!assembly) {
        printf("Could not load '%s' assembly.", file.string().c_str());
        return false;
    }

    image = mono_assembly_get_image(assembly);
    if (!image) {
        printf("Could not load '%s' image.", file.string().c_str());
        return false;
    }

    return true;
}

void PluginAssembly::unload() {
    if (image) {
        mono_image_close(image);
        image = nullptr;
    }

    if (assembly) {
        mono_assembly_close(assembly);
        assembly = nullptr;
    }
}

void PluginAssembly::loadClasses() {
    assert(image);

    pluginClasses.clear();

    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
    int numTypes = mono_table_info_get_rows(typeDefinitionsTable);

    MonoClass* pluginClass = *PluginManager::Get().findCoreClass("Plugin");

    for (int i = 0; i < numTypes; ++i) {
        uint32_t cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

        const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
        const char* className = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
        std::string fullName;
        if (strlen(nameSpace) != 0)
            fullName = nameSpace + "."s + className;
        else
            fullName = className;

        MonoClass* monoClass = mono_class_from_name(image, nameSpace, className);
        if (monoClass == pluginClass)
            continue;

        bool isPlugin = mono_class_is_subclass_of(monoClass, pluginClass, false);
        if (!isPlugin)
            continue;

        pluginClasses[std::move(fullName)] = std::make_shared<PluginClass>(image, nameSpace, className);
    }
}

/*_________________________________________________*/

PluginClass::PluginClass(MonoImage* image, std::string _classNamespace, std::string _className) : classNamespace{std::move(_classNamespace)}, className{std::move(_className)} {
    monoClass = mono_class_from_name(image, classNamespace.c_str(), className.c_str());
}

MonoObject* PluginClass::instantiate() const {
    assert(monoClass);
    return PluginManager::Get().instantiateClass(monoClass);
}

MonoMethod* PluginClass::getMethod(const char* name, int parameterCount) const {
    assert(monoClass);
    return mono_class_get_method_from_name(monoClass, name, parameterCount);
}

MonoMethod* PluginClass::getVirtualMethod(MonoObject* instance, const char* name, int parameterCount) const {
    assert(monoClass);
    MonoMethod* method = mono_class_get_method_from_name(monoClass, name, parameterCount);
    return method ? mono_object_get_virtual_method(instance, method) : nullptr;
}

bool PluginClass::isSubclassOf(MonoClass* otherClass, bool checkInterface) const {
    return mono_class_is_subclass_of(monoClass, otherClass, checkInterface);
}

MonoObject* PluginClass::invokeMethod(MonoObject* instance, MonoMethod* method, void** params) const {
    MonoObject* exception = nullptr;
    return mono_runtime_invoke(method, instance, params, &exception);
}

/*_________________________________________________*/

PluginInstance::PluginInstance(std::string className, fs::path assemblyPath, std::shared_ptr<PluginClass> _pluginClass, PluginId pluginId)
    : name{std::move(className)}, path{std::move(assemblyPath)}, pluginClass{std::move(_pluginClass)}, id{pluginId} {
    instance = pluginClass->instantiate();

    constructor = PluginManager::Get().findCoreClass("Plugin")->getMethod(".ctor", 1);

    onLoad = pluginClass->getMethod("OnLoad", 0);
    onUnload = pluginClass->getMethod("OnUnload", 0);
    onPause = pluginClass->getMethod("OnPause", 0);
    onUnpause = pluginClass->getMethod("OnUnpause", 0);
    onStart = pluginClass->getMethod("OnStart", 0);
    onEnd = pluginClass->getMethod("OnEnd", 0);

    // Call Plugin constructor
    {
        void* param = &id;
        pluginClass->invokeMethod(instance, constructor, &param);
    }

    auto& pluginInfo = *PluginManager::Get().findCoreClass("IPluginInfo");

    if (pluginClass->isSubclassOf(pluginInfo, true)) {
        name = invoke<std::string>(pluginInfo.getVirtualMethod(instance, "GetName", 0));
        description = invoke<std::string>(pluginInfo.getVirtualMethod(instance, "GetDescription", 0));
        url = invoke<std::string>(pluginInfo.getVirtualMethod(instance, "GetURL", 0));
        tag = invoke<std::string>(pluginInfo.getVirtualMethod(instance, "GetTag", 0));
        author = invoke<std::string>(pluginInfo.getVirtualMethod(instance, "GetAuthor", 0));
        licence = invoke<std::string>(pluginInfo.getVirtualMethod(instance, "GetLicence", 0));
        version = invoke<std::string>(pluginInfo.getVirtualMethod(instance, "GetVersion", 0));
        date = invoke<std::string>(pluginInfo.getVirtualMethod(instance, "GetDate", 0));
    }

    auto& pluginListener = *PluginManager::Get().findCoreClass("IPluginListener");

    if (pluginClass->isSubclassOf(pluginListener, true)) {
        onLoaded = pluginListener.getVirtualMethod(instance, "OnLoaded", 1);
        onUnloaded = pluginListener.getVirtualMethod(instance, "OnUnloaded", 1);
        onPaused = pluginListener.getVirtualMethod(instance, "OnPaused", 1);
        onUnpaused = pluginListener.getVirtualMethod(instance, "OnUnpaused", 1);
        onAllLoaded = pluginListener.getVirtualMethod(instance, "OnAllLoaded", 0);
    }
}

bool PluginInstance::invokeOnLoad() {
    bool ret = !onLoad || invoke<bool>(onLoad);
    if (ret) {
        invoke(onStart);
        status = PluginStatus::Running;
    } else {
        status = PluginStatus::Refused;
    }
    return ret;
}

bool PluginInstance::invokeOnUnload(bool force) {
    bool ret = !onUnload || invoke<bool>(onUnload) || force;
    if (ret) {
        invoke(onEnd);
        status = PluginStatus::NotLoaded;
    }
    return ret;
}

bool PluginInstance::invokeOnPause() {
    if (status == PluginStatus::Running) {
        invoke(onPause);
        status = PluginStatus::Paused;
        return true;
    }
    return false;
}

bool PluginInstance::invokeOnUnpause() {
    if (status == PluginStatus::Paused) {
        invoke(onUnpause);
        status = PluginStatus::Running;
        return true;
    }
    return false;
}

/*_________________________________________________*/