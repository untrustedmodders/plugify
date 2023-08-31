#include "plugin_manager.h"
#include "plugin_glue.h"
#include "file_system.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

using namespace std::string_literals;

using namespace wizard;

namespace wizard::utils {
    MonoAssembly* LoadMonoAssembly(const fs::path& assemblyPath, bool loadPDB) {
        MonoImageOpenStatus status = MONO_IMAGE_IMAGE_INVALID;
        MonoImage* image = nullptr;

        FileSystem::ReadBytes<char>(assemblyPath, [&image, &status](std::span<char> buffer) {
            image = mono_image_open_from_data_full(buffer.data(), static_cast<uint32_t>(buffer.size()), 1, &status, 0);
        });

        if (status != MONO_IMAGE_OK) {
            std::cout << "Failed to load assembly file: " << mono_image_strerror(status) << std::endl;
            return nullptr;
        }

        if (loadPDB) {
            fs::path pdbPath{ assemblyPath };
            pdbPath.replace_extension(".pdb");

            FileSystem::ReadBytes<mono_byte>(pdbPath, [&image, &pdbPath](std::span<mono_byte> buffer) {
                mono_debug_open_image_from_memory(image, buffer.data(), static_cast<int>(buffer.size()));
                std::cout << "Loaded PDB: " << pdbPath.string() << std::endl;
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

            std::cout << nameSpace << "." << name << std::endl;
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
            case PluginStatus::Running:
                return "RUNNING";
            default:
                return "-";
        }
    }
}

namespace wizard::dfs {
    void TopSort(const std::string& pluginName, PluginInfoList& sourceList, PluginInfoList& targetList) {
        auto it = std::find_if(sourceList.begin(), sourceList.end(), [&pluginName](const std::unique_ptr<PluginInfo>& i) {
            return i->name == pluginName;
        });
        if (it != sourceList.end()) {
            auto index = static_cast<size_t>(std::distance(sourceList.begin(), it));
            auto info = std::move(sourceList[index]);
            sourceList.erase(it);
            for (const auto& name : info->dependencies) {
                TopSort(name, sourceList, targetList);
            }
            targetList.push_back(std::move(info));
        }
    }

    bool IsCyclicUtil(const std::unique_ptr<PluginInfo>& info, PluginInfoList& sourceList, std::unordered_map<std::string, std::pair<bool, bool>>& data) {
        auto& [visited, recursive] = data[info->name];
        if (!visited) {
            // Mark the current node as visited
            // and part of recursion stack
            visited = true;
            recursive = true;

            // Recur for all the vertices adjacent to this vertex
            for (const auto& name : info->dependencies) {
                auto it = std::find_if(sourceList.begin(), sourceList.end(), [&name](const std::unique_ptr<PluginInfo>& i) {
                    return i->name == name;
                });

                if (it != sourceList.end()) {
                    auto [vis, rec] = data[name];
                    if ((!vis && IsCyclicUtil(*it, sourceList, data)) || rec)
                        return true;
                }
            }
        }

        // Remove the vertex from recursion stack
        recursive = false;
        return false;
    }

    bool IsCyclic(PluginInfoList& sourceList) {
        // Mark all the vertices as not visited
        // and not part of recursion stack
        std::unordered_map<std::string, std::pair<bool, bool>> data; /* [visited, recursive] */

        // Call the recursive helper function
        // to detect cycle in different DFS trees
        for (const auto& obj : sourceList) {
            if (!data[obj->name].first && IsCyclicUtil(obj, sourceList, data))
                return true;
        }

        return false;
    }
}

PluginManager::PluginManager() {
    initMono();

    PluginGlue::RegisterFunctions();
}

PluginManager::~PluginManager() {
    for (size_t i = plugins.size() - 1; i != static_cast<size_t>(-1); --i) {
        plugins[i].invokeOnDestroy();
    }

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
    char appName[] = "WizardMonoRuntime";
    appDomain = mono_domain_create_appdomain(appName, nullptr);
    mono_domain_set(appDomain, true);

    // Load a core assembly
    if (!coreAssembly.load(corePath, enableDebugging))
        throw std::runtime_error("Core assembly '" + corePath.string() + "' is missing!");

    // Retrieve and instantiate core classes
    coreClasses.emplace("Plugin", mono_class_from_name(coreAssembly, "Wizard", "Plugin"));
    coreClasses.emplace("PluginInfo", mono_class_from_name(coreAssembly, "Wizard", "PluginInfo"));

    PluginInfoList pluginInfos;

    // Load a plugin assemblies
    for (auto const& entry : fs::directory_iterator(pluginsPath)) {
        const auto& path = entry.path();
        if (fs::is_regular_file(path) && path.extension().string() == ".dll") {
            PluginAssembly assembly;
            if (assembly.load(path, enableDebugging)) {
                auto info = assembly.loadClass();
                if (info) {
                    info->path = path;
                    pluginInfos.push_back(std::move(info));
                    pluginAssemblies.emplace(path, std::move(assembly));
                }
            }
        }
    }

    if (pluginInfos.empty())
        return;

    // Find plugins with missing dependencies
    for (size_t j = pluginInfos.size() - 1; j != static_cast<size_t>(-1); --j) {
        const auto& info = pluginInfos[j];
        for (const auto& name : info->dependencies) {
            auto it = std::find_if(pluginInfos.begin(), pluginInfos.end(), [&name](const std::unique_ptr<PluginInfo>& i) {
                return i->name == name;
            });
            if (it == pluginInfos.end()) {
                std::cout << "Could not load '" << info->name << "' plugin. It require dependency: '" << name << "' which not exist!" << std::endl;
                pluginAssemblies.erase(info->path);
                pluginInfos.erase(pluginInfos.begin() + static_cast<int64_t>(j));
            }
        }
    }

    // Sort plugins by dependencies
    PluginInfoList sortedInfos;
    sortedInfos.reserve(pluginInfos.size());
    while (!pluginInfos.empty()) {
        dfs::TopSort(pluginInfos.back()->name, pluginInfos, sortedInfos);
    }

    // Function call
    if (dfs::IsCyclic(sortedInfos))
        std::cerr << "Plugins contains cycle dependencies" << std::endl;
    else
        std::cout << "Plugins doesn't contain cycle dependencies" << std::endl;

    // Initialize plugins
    plugins.reserve(sortedInfos.size());

    for (size_t i = 0; i < sortedInfos.size(); ++i) {
        plugins.emplace_back(std::move(sortedInfos[i]), i);
    }
}

PluginInstance* PluginManager::findPlugin(PluginId id) {
    auto size = plugins.size();
    return id < size ? &plugins[id] : nullptr;
}

PluginInstance* PluginManager::findPlugin(std::string_view name) {
    auto it = std::find_if(plugins.begin(), plugins.end(), [name](const PluginInstance& plugin) {
       return plugin.getName() == name;
    });
    return it != plugins.end() ? &*it : nullptr;
}

MonoClass* PluginManager::findCoreClass(const std::string& name) {
    auto it = coreClasses.find(name);
    return it != coreClasses.end() ? it->second : nullptr;
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
    other.assembly = nullptr;
    other.image = nullptr;
}

bool PluginAssembly::load(const fs::path& path, bool enableDebugging) {
    unload();

    assembly = utils::LoadMonoAssembly(path, enableDebugging);
    if (!assembly) {
        std::cout << "Could not load '" << path.string() << "' assembly." << std::endl;
        return false;
    }

    image = mono_assembly_get_image(assembly);
    if (!image) {
        std::cout << "Could not load '" << path.string() << "' image." << std::endl;
        return false;
    }

    return true;
}

void PluginAssembly::unload() {
    if (!PluginManager::Get().appDomain)
        return;

    if (image) {
        ///mono_image_close(image);
        image = nullptr;
    }

    if (assembly) {
        ///mono_assembly_close(assembly);
        assembly = nullptr;
    }
}

std::unique_ptr<PluginInfo> PluginAssembly::loadClass() {
    assert(image);

    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
    int numTypes = mono_table_info_get_rows(typeDefinitionsTable);

    MonoClass* pluginClass = PluginManager::Get().findCoreClass("Plugin");
    MonoClass* pluginInfo = PluginManager::Get().findCoreClass("PluginInfo");
    MonoDomain* appDomain = PluginManager::Get().appDomain;

    for (int i = 0; i < numTypes; ++i) {
        uint32_t cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

        const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
        const char* className = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

        MonoClass* monoClass = mono_class_from_name(image, nameSpace, className);
        if (monoClass == pluginClass)
            continue;

        bool isPlugin = mono_class_is_subclass_of(monoClass, pluginClass, false);
        if (!isPlugin)
            continue;

        auto info = std::make_unique<PluginInfo>(monoClass);

        MonoCustomAttrInfo* attributes = mono_custom_attrs_from_class(monoClass);
        if (attributes) {
            for (int j = 0; j < attributes->num_attrs; ++j) {
                auto& attrs = attributes->attrs[j];
                if (pluginInfo != mono_method_get_class(attrs.ctor))
                    continue;

                auto instance = mono_custom_attrs_get_attr(attributes, pluginInfo);

                void* iterator = nullptr;
                while (MonoClassField* field = mono_class_get_fields(pluginInfo, &iterator)) {
                    const char* fieldName = mono_field_get_name(field);
                    MonoObject* fieldValue = mono_field_get_value_object(appDomain, field, instance);

                    if (!strcmp(fieldName, "_name"))
                        info->name = utils::MonoStringToString((MonoString*) fieldValue);
                    else if (!strcmp(fieldName, "_description"))
                        info->description = utils::MonoStringToString((MonoString*) fieldValue);
                    else if (!strcmp(fieldName, "_author"))
                        info->author = utils::MonoStringToString((MonoString*) fieldValue);
                    else if (!strcmp(fieldName, "_version"))
                        info->version = utils::MonoStringToString((MonoString*) fieldValue);
                    else if (!strcmp(fieldName, "_url"))
                        info->url = utils::MonoStringToString((MonoString*) fieldValue);
                    else if (!strcmp(fieldName, "_dependencies")) {
                        MonoArray* array = (MonoArray*) fieldValue;
                        uintptr_t length = mono_array_length(array);
                        info->dependencies.reserve(length);
                        for (uintptr_t k = 0; k < length; ++k) {
                            MonoString* str = mono_array_get(array, MonoString*, k);
                            if (mono_string_length(str) > 0)
                                info->dependencies.push_back(utils::MonoStringToString(str));
                        }
                    }
                }
                break;
            }
            mono_custom_attrs_free(attributes);
        }

        if (info->name.empty()) {
            if (strlen(nameSpace) != 0)
                info->name = nameSpace + "."s + className;
            else
                info->name = className;
        }

        return info;
    }

    return nullptr;
}

/*_________________________________________________*/

PluginInstance::PluginInstance(std::unique_ptr<PluginInfo>&& pluginInfo, PluginId pluginId) : info{std::move(pluginInfo)}, id{pluginId} {
    instance = PluginManager::Get().instantiateClass(info->monoClass);

    // Call Plugin constructor
    {
        MonoClass* pluginClass = PluginManager::Get().findCoreClass("Plugin");
        MonoMethod* constructor = mono_class_get_method_from_name(pluginClass, ".ctor", 1);

        void* param = &id;
        MonoObject* exception = nullptr;
        mono_runtime_invoke(constructor, instance, &param, &exception);
        //TODO: Handle exception
    }

    onCreateMethod = mono_class_get_method_from_name(info->monoClass, "OnCreate", 0);
    onUpdateMethod = mono_class_get_method_from_name(info->monoClass, "OnUpdate", 1);
    onDestroyMethod = mono_class_get_method_from_name(info->monoClass, "OnDestroy", 0);

    status = PluginStatus::Running;

    invokeOnCreate();
}

void PluginInstance::invokeOnCreate() const {
    if (onCreateMethod) {
        MonoObject* exception = nullptr;
        mono_runtime_invoke(onCreateMethod, instance, nullptr, &exception);
        //TODO: Handle exception
    }
}

void PluginInstance::invokeOnUpdate(float ts) const {
    if (onUpdateMethod) {
        void* param = &ts;
        MonoObject* exception = nullptr;
        mono_runtime_invoke(onUpdateMethod, instance, &param, &exception);
        //TODO: Handle exception
    }
}

void PluginInstance::invokeOnDestroy() const {
    if (onDestroyMethod) {
        MonoObject* exception = nullptr;
        mono_runtime_invoke(onDestroyMethod, instance, nullptr, &exception);
        //TODO: Handle exception
    }
}

/*_________________________________________________*/

PluginInfo::PluginInfo(MonoClass* _monoClass) : monoClass{_monoClass} {
}