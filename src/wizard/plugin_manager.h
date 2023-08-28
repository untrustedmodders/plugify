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

    enum {
        BadLoad = 0,
        Console = -1,
        File = -2,
        MinId = 1,
    };

    enum class PluginStatus {
        NotLoaded = -4,
        Error = -3,
        Refused = -2,
        Paused = -1,
        Running = 0
    };

    namespace Utils {
        MonoAssembly* LoadMonoAssembly(const fs::path& assemblyPath, bool loadPDB = false);
        void PrintAssemblyTypes(MonoAssembly* assembly);
        std::string MonoStringToString(MonoString* string);
        const char* GetStatusText(PluginStatus status);
    }

    class PluginClass  {
    public:
        PluginClass() = default;
        PluginClass(MonoImage* image, std::string classNamespace, std::string className);
        PluginClass(PluginClass const&) = delete;
        PluginClass& operator=(PluginClass const&) = delete;

        MonoObject* instantiate() const;

        MonoMethod* getMethod(const char* name, int parameterCount) const;
        MonoMethod* getVirtualMethod(MonoObject* instance, const char* name, int parameterCount) const;

        bool isSubclassOf(MonoClass* monoClass, bool checkInterface = false) const;

        MonoObject* invokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr) const;

        operator bool() const { return monoClass != nullptr; }
        operator MonoClass*() const { return monoClass; }
        MonoClass* getClass() { return monoClass; }

    private:
        MonoClass* monoClass{ nullptr };

        std::string classNamespace;
        std::string className;

        friend class PluginManager;
    };

    class PluginInstance {
    public:
        PluginInstance() = delete;
        PluginInstance(std::string className, fs::path assemblyPath, std::shared_ptr<PluginClass> pluginClass, PluginId pluginId);
        PluginInstance(PluginInstance const&) = delete;
        PluginInstance& operator=(PluginInstance const&) = delete;

        std::shared_ptr<PluginClass> getPluginClass() const { return pluginClass; }
        PluginStatus getStatus() const { return status; }

        uint64_t getId() const { return id; }
        const std::string& getName() const { return name; }
        const std::string& getDescription() const { return description; }
        const std::string& getUrl() const { return url; }
        const std::string& getTag() const { return tag; }
        const std::string& getAuthor() const { return author; }
        const std::string& getLicence() const { return licence; }
        const std::string& getVersion() const { return version; }
        const std::string& getDate() const { return date; }
        const fs::path& getPath() const { return path; }

        operator bool() const { return instance != nullptr; }
        operator MonoObject*() const { return instance; }
        MonoObject* getManagedObject() const { return instance; }

    private:
        bool invokeOnLoad();
        bool invokeOnUnload(bool force);
        bool invokeOnPause();
        bool invokeOnUnpause();

        template<typename M>
        void invoke(M&& method, void** params = nullptr) {
            if (method)
                pluginClass->invokeMethod(instance, method, params);
        }

        template<typename T, typename M>
        T invoke(M&& method, void** params = nullptr) {
            if (method) {
                if constexpr (std::is_same_v<T, std::string>)
                    return Utils::MonoStringToString((MonoString*)pluginClass->invokeMethod(instance, method, params));
                else
                    return *reinterpret_cast<T*>(mono_object_unbox(pluginClass->invokeMethod(instance, method, params)));
            }
            return T{};
        }

    private:
        std::shared_ptr<PluginClass> pluginClass;
        PluginStatus status{ PluginStatus::NotLoaded };

        uint64_t id;

        MonoObject* instance{ nullptr };
        MonoMethod* constructor{ nullptr };

        // Plugin
        MonoMethod* onLoad{ nullptr };
        MonoMethod* onUnload{ nullptr };
        MonoMethod* onPause{ nullptr };
        MonoMethod* onUnpause{ nullptr };
        MonoMethod* onStart{ nullptr };
        MonoMethod* onEnd{ nullptr };

        // IPluginListener
        MonoMethod* onLoaded{ nullptr };
        MonoMethod* onUnloaded{ nullptr };
        MonoMethod* onPaused{ nullptr };
        MonoMethod* onUnpaused{ nullptr };
        MonoMethod* onAllLoaded{ nullptr };

        // IPluginInfo
        std::string name;
        std::string description;
        std::string url;
        std::string tag;
        std::string author;
        std::string licence;
        std::string version;
        std::string date;

        fs::path path;

        friend class PluginManager;
    };

    using PluginClassesPtrMap = std::unordered_map<std::string, std::shared_ptr<PluginClass>>;

    class PluginAssembly {
    public:
        PluginAssembly() = default;
        ~PluginAssembly();
        PluginAssembly(PluginAssembly&&) noexcept;
        PluginAssembly(PluginAssembly const&) = delete;
        PluginAssembly& operator=(PluginAssembly const&) = delete;
        PluginAssembly& operator=(PluginAssembly&&) noexcept;

        bool load(const fs::path& file, bool enableDebugging);
        void unload();

        void loadClasses();

        operator bool() const { return image != nullptr; }
        operator MonoImage*() const { return image; }
        MonoImage* getImage() const { return image; }

        const PluginClassesPtrMap& getPluginClasses() const { return pluginClasses; }

    private:
        MonoAssembly* assembly{ nullptr };
        MonoImage* image{ nullptr };

        PluginClassesPtrMap pluginClasses;

        friend class PluginManager;
    };

    using PluginAssemblyMap = std::unordered_map<fs::path, PluginAssembly>;
    using PluginClassesMap = std::unordered_map<std::string, PluginClass>;
    using PluginMap = std::map<PluginId, PluginInstance>;

    class PluginManager {
    private:
        PluginManager();
        ~PluginManager();

    public:
        static PluginManager& Get();

        void loadAll();
        //void unloadAll();

        std::vector<PluginId> load(const fs::path& file, bool& already);
        std::vector<PluginId> unload(const fs::path& file, bool force);
        bool pause(PluginId id);
        bool unpause(PluginId id);

        const PluginAssemblyMap& getAssemblies() const { return pluginAssemblies; }
        const PluginMap& getPlugins() const { return plugins; }
        size_t getPluginCount() { return plugins.size(); }

        PluginInstance* findPlugin(PluginId id);

        PluginMap::iterator begin() { return plugins.begin(); }
        PluginMap::iterator end() { return plugins.end(); }
        PluginMap::reverse_iterator rbegin() { return plugins.rbegin(); }
        PluginMap::reverse_iterator rend() { return plugins.rend(); }
        PluginMap::const_iterator begin() const { return plugins.begin(); }
        PluginMap::const_iterator end() const { return plugins.end(); }
        PluginMap::const_reverse_iterator rbegin() const { return plugins.rbegin(); }
        PluginMap::const_reverse_iterator rend() const { return plugins.rend(); }

    private:
        void initMono();
        void shutdownMono();

        MonoString* createString(const char* string) const;
        MonoObject* instantiateClass(MonoClass* monoClass) const;

        PluginClass* findCoreClass(const std::string& name);

    private:
        MonoDomain* rootDomain{ nullptr };
        MonoDomain* appDomain{ nullptr };

        fs::path corePath{ "../addons/wizard/bin/Wizard.dll" };
        fs::path pluginsPath{ "../addons/wizard/plugins/" };

        PluginAssembly coreAssembly;
        PluginAssemblyMap pluginAssemblies;
        PluginClassesMap coreClasses;

        PluginMap plugins;
        PluginId lastPluginId{ MinId };

        friend class PluginClass;
        friend class PluginAssembly;
        friend class PluginInstance;

#ifdef _DEBUG
        bool enableDebugging{ true };
#else
        bool enableDebugging{ false };
#endif
    };
}