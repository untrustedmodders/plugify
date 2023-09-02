#pragma once

extern "C" {
    typedef struct _MonoClass MonoClass;
    typedef struct _MonoObject MonoObject;
    typedef struct _MonoMethod MonoMethod;
    typedef struct _MonoAssembly MonoAssembly;
    typedef struct _MonoImage MonoImage;
    typedef struct _MonoClassField MonoClassField;
    typedef struct _MonoString MonoString;
    typedef struct _MonoDomain MonoDomain;
}

namespace wizard {
    using PluginId = uint64_t;

    enum class PluginStatus {
        NotLoaded = -1,
        Error = 0,
        Running = 1
    };

    namespace utils {
        MonoAssembly* LoadMonoAssembly(const fs::path& assemblyPath, bool loadPDB = false);
        void PrintAssemblyTypes(MonoAssembly* assembly);
        std::string MonoStringToString(MonoString* string);
        const char* GetStatusText(PluginStatus status);
    }

    struct PluginInfo {
        MonoAssembly* assembly{ nullptr };
        MonoImage* image{ nullptr };
        MonoClass* klass{ nullptr };

        std::string name;
        std::string description;
        std::string author;
        std::string version;
        std::string url;
        std::vector<std::string> dependencies;
        fs::path path;

        PluginInfo(MonoAssembly* assembly, MonoImage* image, MonoClass* klass);
    };

    class PluginInstance {
    public:
        PluginInstance() = default;
        PluginInstance(std::unique_ptr<PluginInfo>&& pluginInfo, PluginId pluginId);
        PluginInstance(PluginInstance&& other) noexcept = default;
        ~PluginInstance() = default;

        void invokeOnCreate() const;
        void invokeOnUpdate(float ts) const;
        void invokeOnDestroy() const;

        uint64_t getId() const { return id; }
        const std::string& getName() const { return info->name; }
        const std::string& getDescription() const { return info->description; }
        const std::string& getUrl() const { return info->url; }
        const std::string& getAuthor() const { return info->author; }
        const std::string& getVersion() const { return info->version; }
        const fs::path& getPath() const { return info->path; }

        operator bool() const { return instance != nullptr; }
        operator MonoObject*() const { return instance; }
        MonoObject* getManagedObject() const { return instance; }

        PluginStatus getStatus() const { return status; }

    private:
        std::unique_ptr<PluginInfo> info;
        uint64_t id;

        MonoObject* instance{ nullptr };
        //MonoMethod* constructor{ nullptr };
        MonoMethod* onCreateMethod{ nullptr };
        MonoMethod* onUpdateMethod{ nullptr };
        MonoMethod* onDestroyMethod{ nullptr };

        PluginStatus status{ PluginStatus::NotLoaded };

        friend class PluginManager;
    };

    using PluginInfoList = std::vector<std::unique_ptr<PluginInfo>>;
    using PluginList = std::vector<PluginInstance>;

    class PluginManager {
    private:
        PluginManager();
        ~PluginManager();

    public:
        static PluginManager& Get();

        void loadAll();
        //void unloadAll();

        const PluginList& getPlugins() const { return plugins; }
        size_t getPluginCount() { return plugins.size(); }

        PluginList::iterator begin() { return plugins.begin(); }
        PluginList::iterator end() { return plugins.end(); }
        PluginList::reverse_iterator rbegin() { return plugins.rbegin(); }
        PluginList::reverse_iterator rend() { return plugins.rend(); }
        PluginList::const_iterator begin() const { return plugins.begin(); }
        PluginList::const_iterator end() const { return plugins.end(); }
        PluginList::const_reverse_iterator rbegin() const { return plugins.rbegin(); }
        PluginList::const_reverse_iterator rend() const { return plugins.rend(); }

        PluginInstance* findPlugin(PluginId id);
        PluginInstance* findPlugin(std::string_view name);

    private:
        void initMono();
        void shutdownMono();
		
		std::unique_ptr<PluginInfo> loadPlugin(const fs::path& path);
		
        MonoString* createString(const char* string) const;
        MonoObject* instantiateClass(MonoClass* klass) const;

        MonoClass* findCoreClass(const std::string& name);
        MonoMethod* findCoreMethod(const std::string& name);

    private:
        MonoDomain* rootDomain{ nullptr };
        MonoDomain* appDomain{ nullptr };

        MonoAssembly* coreAssembly{ nullptr };
        MonoImage* coreImage{ nullptr };

        std::unordered_map<std::string, MonoClass*> coreClasses;
        std::unordered_map<std::string, MonoMethod*> coreMethods;

        PluginList plugins;

        fs::path corePath{ "../addons/wizard/bin/Wizard.dll" };
        fs::path pluginsPath{ "../addons/wizard/plugins/" };

        friend class PluginInstance;

#ifdef _DEBUG
        bool enableDebugging{ true };
#else
        bool enableDebugging{ false };
#endif
    };
}