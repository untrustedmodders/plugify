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
	coreAssembly = utils::LoadMonoAssembly(corePath, enableDebugging);
    if (!coreAssembly) {
        std::cout << "Could not load '" << corePath.string() << "' core assembly." << std::endl;
        return;
    }

    coreImage = mono_assembly_get_image(coreAssembly);
    if (!coreImage) {
        std::cout << "Could not load '" << corePath.string() << "' core image." << std::endl;
        return;
    }

    // Retrieve and cache core classes/methods
	
	/// Plugin
    MonoClass* pluginClass = cacheCoreClass("Plugin");
	cacheCoreMethod(pluginClass, ".ctor", 1);
	
	/// PluginInfo
	cacheCoreClass("PluginInfo");
	
	/// IServerListener
	{
		MonoClass* serverListener = cacheCoreClass("IServerListener");
		cacheCoreMethod(serverListener, "OnConfigsExecuted", 0);
		cacheCoreMethod(serverListener, "OnLevelShutdown", 0);
		cacheCoreMethod(serverListener, "OnLevelInit", 1);
		cacheCoreMethod(serverListener, "OnLevelStart", 0);
		cacheCoreMethod(serverListener, "OnEntityCreated", 2);
		cacheCoreMethod(serverListener, "OnEntityDestroyed", 2);
	}

	/// IClientListener
	{
		MonoClass* clientListener = cacheCoreClass("IClientListener");
		cacheCoreMethod(clientListener, "OnClientAuthorized", 2);
		cacheCoreMethod(clientListener, "OnClientCommand", 2);
		cacheCoreMethod(clientListener, "OnClientConnect", 4);
		cacheCoreMethod(clientListener, "OnClientConnected", 1);
		cacheCoreMethod(clientListener, "OnClientDisconnect", 1);
		cacheCoreMethod(clientListener, "OnClientDisconnected", 1);
		cacheCoreMethod(clientListener, "OnClientPutInServer", 2);
		cacheCoreMethod(clientListener, "OnClientActive", 2);
		cacheCoreMethod(clientListener, "OnClientSettingsChanged", 1);
	}
	
	//
	
    PluginInfoList pluginInfos;

    // Load a plugin assemblies
    for (auto const& entry : fs::directory_iterator(pluginsPath)) {
        const auto& path = entry.path();
        if (fs::is_regular_file(path) && path.extension().string() == ".dll") {
			if (auto info = loadPlugin(path))
				pluginInfos.push_back(std::move(info));
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
    return id < plugins.size() ? &plugins[id] : nullptr;
}

PluginInstance* PluginManager::findPlugin(std::string_view name) {
    auto it = std::find_if(plugins.begin(), plugins.end(), [name](const PluginInstance& plugin) {
       return plugin.getName() == name;
    });
    return it != plugins.end() ? &*it : nullptr;
}

MonoClass* PluginManager::cacheCoreClass(const char* name) {
	MonoClass* klass = mono_class_from_name(coreImage, "Wizard", name);
	assert(klass);
	coreClasses.emplace(name, klass);
	return klass;
}
MonoMethod* PluginManager::cacheCoreMethod(MonoClass* klass, const char* name, int params) {
	MonoMethod* method = mono_class_get_method_from_name(klass, name, params);
	assert(method);
	coreMethods.emplace(name, method);
	return method;
}

MonoClass* PluginManager::findCoreClass(const std::string& name) {
    auto it = coreClasses.find(name);
    return it != coreClasses.end() ? it->second : nullptr;
}

MonoMethod* PluginManager::findCoreMethod(const std::string& name) {
    auto it = coreMethods.find(name);
    return it != coreMethods.end() ? it->second : nullptr;
}

MonoString* PluginManager::createString(const char* string) const {
    return mono_string_new(appDomain, string);
}

MonoObject* PluginManager::instantiateClass(MonoClass* klass) const {
    MonoObject* instance = mono_object_new(appDomain, klass);
    mono_runtime_object_init(instance);
    return instance;
}

std::unique_ptr<PluginInfo> PluginManager::loadPlugin(const fs::path& path) {
    MonoAssembly* assembly = utils::LoadMonoAssembly(path, enableDebugging);
    if (!assembly) {
        std::cout << "Could not load '" << path.string() << "' assembly." << std::endl;
        return nullptr;
    }

    MonoImage* image = mono_assembly_get_image(assembly);
    if (!image) {
        std::cout << "Could not load '" << path.string() << "' image." << std::endl;
        return nullptr;
    }

    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
    int numTypes = mono_table_info_get_rows(typeDefinitionsTable);

    MonoClass* pluginClass = findCoreClass("Plugin");
    MonoClass* pluginInfo = findCoreClass("PluginInfo");

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

        auto info = std::make_unique<PluginInfo>(assembly, image, monoClass);

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
		
		info->path = path;

        return info;
    }

    return nullptr;
}

/*_________________________________________________*/

PluginInstance::PluginInstance(std::unique_ptr<PluginInfo>&& pluginInfo, PluginId pluginId) : info{std::move(pluginInfo)}, id{pluginId} {
    MonoClass* monoClass = info->klass;
	
	instance = PluginManager::Get().instantiateClass();

    // Call Plugin constructor
    {
        MonoMethod* constructor = PluginManager::Get().findCoreMethod(".ctor");
		assert(constructor);
        void* param = &id;
        MonoObject* exception = nullptr;
        mono_runtime_invoke(constructor, instance, &param, &exception);
        //TODO: Handle exception
    }

	cacheMethod("OnCreate", 0);
	cacheMethod("OnUpdate", 1);
	cacheMethod("OnDestroy", 0);

	// IServerListener
	MonoClass* serverListener = PluginManager::Get().findCoreClass("IServerListener");
	if (mono_class_is_subclass_of(monoClass, serverListener, true)) {
		cacheVirtualMethod("OnConfigsExecuted");
		cacheVirtualMethod("OnLevelShutdown");
		cacheVirtualMethod("OnLevelInit");
		cacheVirtualMethod("OnLevelStart");
		cacheVirtualMethod("OnEntityCreated");
		cacheVirtualMethod("OnEntityDestroyed");
	}
	
	// IClientListener
	MonoClass* clientListener = PluginManager::Get().findCoreClass("IClientListener");
	if (mono_class_is_subclass_of(monoClass, clientListener, true)) {
		cacheVirtualMethod("OnClientAuthorized");
		cacheVirtualMethod("OnClientCommand");
		cacheVirtualMethod("OnClientConnect");
		cacheVirtualMethod("OnClientConnected");
		cacheVirtualMethod("OnClientDisconnect");
		cacheVirtualMethod("OnClientPutInServer");
		cacheVirtualMethod("OnClientActive");
		cacheVirtualMethod("OnClientSettingsChanged");
	}

    status = PluginStatus::Running;

    invokeOnCreate();
}

void PluginInstance::invokeOnCreate() const {
	if (auto it = methods.find("OnCreate"); it != methods.ends()) {
        MonoObject* exception = nullptr;
        mono_runtime_invoke(it->second, instance, nullptr, &exception);
        //TODO: Handle exception
    }
}

void PluginInstance::invokeOnUpdate(float ts) const {
    if (auto it = methods.find("OnUpdate"); it != methods.ends()) {
        void* param = &ts;
        MonoObject* exception = nullptr;
        mono_runtime_invoke(it->second, instance, &param, &exception);
        //TODO: Handle exception
    }
}

void PluginInstance::invokeOnDestroy() const {
    if (auto it = methods.find("OnDestroy"); it != methods.ends()) {
        MonoObject* exception = nullptr;
        mono_runtime_invoke(it->second, instance, nullptr, &exception);
        //TODO: Handle exception
    }
}

MonoMethod* PluginInstance::cacheMethod(const char* name, int params) {
	Method* method = mono_class_get_method_from_name(monoClass, name, params);
	assert(method);
	methods.emplace(name, method);
	return method;
}

MonoMethod* PluginInstance::cacheVirtualMethod(const char* name) {
	Method* method = PluginManager::Get().findCoreMethod(name);
	assert(method);
	Method* vmethod = mono_object_get_virtual_method(instance, method);
	assert(vmethod);
	methods.emplace(name, vmethod);
	return vmethod;
}

/*_________________________________________________*/

PluginInfo::PluginInfo(MonoAssembly* _assembly, MonoImage* _image, MonoClass* _klass)
    : assembly{_assembly}, image{_image}, klass{_klass} {
}