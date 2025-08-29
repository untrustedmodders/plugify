#pragma once

#include "plugify/core/assembly.hpp"
#include "plugify/core/extension.hpp"
#include "plugify/core/language_module.hpp"
#include "plugify/core/provider.hpp"

namespace plugify {
    template<typename Callback>
    class ScopedTimer {
    public:
        explicit ScopedTimer(Callback&& cb) noexcept(std::is_nothrow_move_constructible_v<Callback>)
            : m_callback(std::forward<Callback>(cb)), m_start(Clock::now()) {}

        ~ScopedTimer() noexcept(std::is_nothrow_invocable_v<Callback>) {
            auto end = Clock::now();
            auto elapsed = std::chrono::duration_cast<Duration>(end - m_start);
            m_callback(elapsed);
        }

        // Non-copyable
        ScopedTimer(const ScopedTimer&) = delete;
        ScopedTimer& operator=(const ScopedTimer&) = delete;

        // Movable
        ScopedTimer(ScopedTimer&& other) noexcept
            : m_callback(std::move(other.m_callback)), m_start(other.m_start) {}

        ScopedTimer& operator=(ScopedTimer&& other) noexcept {
            if (this != &other) {
                m_callback = std::move(other.m_callback);
                m_start = other.m_start;
            }
            return *this;
        }

    private:
        Callback m_callback;
        TimePoint m_start;
    };

    // Deduction guide (important!)
    template <typename Callback>
    ScopedTimer(Callback&&) -> ScopedTimer<std::decay_t<Callback>>;

    /*struct Loader {
        // void InitiailizeModule(Extension& module)
        Result<void> LoadModule(std::shared_ptr<Provider> provider, Extension& module);
        void UpdateModule(Extension& module, double deltaTime);
        void UnloadModule(Extension& module);
        // void TerminateModule(Extension& module)
        //void UpdateModules(const std::vector<Extension&>& modules, double deltaTime)

        Result<void> LoadPlugin(Extension& module, Extension& plugin);
        void StartPlugin(Extension& plugin);
        void UpdatePlugin(Extension& plugin, double deltaTime);
        void EndPlugin(Extension& plugin);
        void UnloadPlugin(Extension& plugin);
        //void UpdatePlugins(const std::vector<Extension&>& plugins, double deltaTime);

        void MethodExport(Extension& plugin);
    };*/

    // Advanced: Exception-safe RAII wrapper for plugin calls
    class SafeCall {
        std::string_view operation;
        std::string_view extensionName;

    public:
        SafeCall(std::string_view op, std::string_view name)
            : operation(op), extensionName(name) {}

        template<typename T, typename Func>
        Result<T> Execute(Func&& func) noexcept {
            try {
                return func();
            } catch (const std::bad_alloc&) {
                return plg::unexpected(std::format("{}: out of memory", operation));
            } catch (const std::exception& e) {
                return plg::unexpected(std::format("{} failed for '{}': {}", operation, extensionName, e.what()));
            } catch (...) {
                return plg::unexpected(std::format("{} failed for '{}': unknown exception",  operation, extensionName));
            }
        }
    };

    // Enhanced Loader class with better error handling and preloading support
    class Loader {
    public:
        struct LoadStatistics {
            size_t modulesLoaded{0};
            size_t pluginsLoaded{0};
            Duration totalLoadTime{};
            Duration slowestLoad{};
            std::string slowestExtension;
        };

    private:
        std::shared_ptr<Provider> _provider;
        using Dependencies = ServiceContext<ILogger, IFileSystem, IAssemblyLoader>;
        Dependencies _deps;
        LoadStatistics _stats;

    public:
        Loader(std::shared_ptr<Context> context, std::shared_ptr<Manager> manager)
            : _deps(context->GetServices())
            , _provider()
        {
            if (!_deps.IsValid()) {
                throw std::runtime_error("Missing required services");
            }
        }

        // Module Operations
        Result<void> LoadModule(Extension& module) {
            auto timer = ScopedTimer([&](Duration elapsed) {
                _stats.totalLoadTime += elapsed;
                if (elapsed > _stats.slowestLoad) {
                    _stats.slowestLoad = elapsed;
                    _stats.slowestExtension = module.GetName();
                }
            });

            // Try to use preloaded assembly if available
            auto assemblyResult = GetOrLoadAssembly(module.GetRuntime(), module.GetDirectories());
            if (!assemblyResult) {
                return plg::unexpected(std::move(assemblyResult.error()));
            }

            // Load language module interface
            auto langModuleResult = LoadLanguageModule(*assemblyResult, module);
            if (!langModuleResult) {
                return langModuleResult;
            }

            _stats.modulesLoaded++;
            return {};
        }

        Result<void> UnloadModule(Extension& module) {
            Result<void> result;

            if (auto* languageModule = module.GetLanguageModule()) {
                result = SafeCall("Shutdown", module.GetName()).Execute<void>([&] {
                    languageModule->Shutdown();
                    return Result<void>{};
                });
                module.SetLanguageModule(nullptr);
            }

            // Clear assembly and remove from cache
            if (auto assembly = module.GetAssembly()) {
                module.SetAssembly(nullptr);
            }

            return result;
        }

        // Plugin Operations
        Result<void> LoadPlugin(const Extension& module, Extension& plugin) {
            auto timer = ScopedTimer([&](Duration elapsed) {
                _stats.totalLoadTime += elapsed;
                if (elapsed > _stats.slowestLoad) {
                    _stats.slowestLoad = elapsed;
                    _stats.slowestExtension = plugin.GetName();
                }
            });

            // Validate module is loaded
            if (module.CanLoad()) {
                return plg::unexpected("Module not loaded");
            }

            auto* languageModule = module.GetLanguageModule();
            if (!languageModule) {
                return plg::unexpected("Language module not available");
            }

            // Load plugin through language module
            auto loadResult = SafeCall("OnPluginLoad", plugin.GetName()).Execute<LoadData>([&] {
                return plugin.GetLanguageModule()->OnPluginLoad(plugin);
            });
            if (!loadResult) {
                return plg::unexpected(std::move(loadResult.error()));
            }

            // Validate and set plugin data
            auto validateResult = ValidateAndSetPluginData(plugin, languageModule, *loadResult);
            if (!validateResult) {
                return validateResult;
            }

            _stats.pluginsLoaded++;
            return {};
        }

        Result<void> StartPlugin(const Extension& plugin) {
            const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin.GetMethodTable();
            if (!hasStart) {
                return {};
            }
            return SafeCall("OnPluginStart", plugin.GetName()).Execute<void>([&] {
                plugin.GetLanguageModule()->OnPluginStart(plugin);
                return Result<void>{};
            });
        }

        Result<void> EndPlugin(const Extension& plugin) {
            const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin.GetMethodTable();
            if (!hasEnd) {
                return {};
            }
            return SafeCall("OnPluginEnd", plugin.GetName()).Execute<void>([&] {
                plugin.GetLanguageModule()->OnPluginEnd(plugin);
                return Result<void>{};
            });
        }

        void UnloadPlugin(Extension& plugin) {
            // Clear all plugin data
            plugin.SetLanguageModule(nullptr);
            plugin.SetUserData(nullptr);
            plugin.SetMethodsData({});
        }

        // Batch operations for efficiency
        void Update(std::span<const Extension> extensions, Duration deltaTime) {
            for (auto& extension : extensions) {
                if (!extension.CanUpdate()) continue;
                const auto& [hasUpdate, hasStart, hasEnd, hasExport] = extension.GetMethodTable();
                if (hasUpdate) {
                    Result<void> result;
                    switch (extension.GetType()) {
                        case ExtensionType::Module:
                            result = SafeCall("OnUpdate", extension.GetName()).Execute<void>([&] {
                                extension.GetLanguageModule()->OnUpdate(deltaTime);
                                return Result<void>{};
                            });
                            break;
                        case ExtensionType::Plugin:
                            result = SafeCall("OnPluginUpdate", extension.GetName()).Execute<void>([&] {
                                extension.GetLanguageModule()->OnPluginUpdate(extension, deltaTime);
                                return Result<void>{};
                            });
                            break;
                        default:
                            result = std::unexpected("Unknown extension type");
                            break;
                    }
                    if (!result) {
                        _deps.Get<ILogger>()->Log(result.error(), Severity::Error);
                    }
                }
            }
        }

        Result<void> MethodExport(const Extension& module, const Extension& plugin) {
            const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin.GetMethodTable();
            if (!hasExport) {
                return {};
            }
            return SafeCall("OnMethodExport", module.GetName()).Execute<void>([&] {
                module.GetLanguageModule()->OnMethodExport(plugin);
                return Result<void>{};
            });
        }

        // Statistics
        const LoadStatistics& GetStatistics() const { return _stats; }
        void ResetStatistics() { _stats = {}; }

    private:
        // Helper to get or load assembly
        Result<std::shared_ptr<IAssembly>> GetOrLoadAssembly(
            const std::filesystem::path& path,
            const std::vector<std::filesystem::path>& searchPaths
        ) {
            auto absPath = _deps.Get<IFileSystem>()->GetAbsolutePath(path);
            if (!absPath) {
                return plg::unexpected(std::move(absPath.error()));
            }

            if (_deps.Get<IAssemblyLoader>()->CanLinkSearchPaths()) {
                for (const auto& searchPath : searchPaths) {
                    _deps.Get<IAssemblyLoader>()->AddSearchPath(searchPath);
                }
            }

            // Load assembly
            LoadFlag flags = GetLoadFlags();
            auto assemblyResult = _deps.Get<IAssemblyLoader>()->Load(*absPath, flags);
            if (!assemblyResult) {
                return plg::unexpected(std::move(assemblyResult.error()));
            }

            return *assemblyResult;
        }

        LoadFlag GetLoadFlags() {
            LoadFlag flags = LoadFlag::Lazy | LoadFlag::Global | LoadFlag::SearchUserDirs | LoadFlag::SearchSystem32 | LoadFlag::SearchDllLoadDir;
            if (_provider->IsPreferOwnSymbols()) {
                flags |= LoadFlag::Deepbind;
            }
            return flags;
        }

        /*LoadFlag GetPreLoadFlags() {
            LoadFlag flags = LoadFlag::Lazy | LoadFlag::DontResolveDllReferences;
            return flags;
        }*/

        Result<void> LoadLanguageModule(
            std::shared_ptr<IAssembly> assembly,
            Extension& module)
        {
            constexpr std::string_view kGetLanguageModuleFn = "GetLanguageModule";

            auto GetLanguageModuleFunc = assembly->GetSymbol(kGetLanguageModuleFn).RCast<ILanguageModule*(*)()>();
            if (!GetLanguageModuleFunc) {
                return plg::unexpected(std::format("Function '{}' not found", kGetLanguageModuleFn));
            }

            auto* languageModule = GetLanguageModuleFunc();
            if (!languageModule) {
                return plg::unexpected(std::format("Invalid address from '{}'", kGetLanguageModuleFn));
            }

    #if PLUGIFY_PLATFORM_WINDOWS
            constexpr bool plugifyBuildType = PLUGIFY_IS_DEBUG;
            bool moduleBuildType = languageModule->IsDebugBuild();
            if (moduleBuildType != plugifyBuildType) {
                return plg::unexpected(std::format(
                    "Build type mismatch: plugify={}, module={}",
                    plugifyBuildType ? "debug" : "release",
                    moduleBuildType ? "debug" : "release"));
            }
    #endif

            auto initResult = SafeCall("Initialize", module.GetName()).Execute<InitData>([&] {
                return languageModule->Initialize(_provider, module);
            });

            if (!initResult) {
                return plg::unexpected(std::move(initResult.error()));
            }

            module.SetLanguageModule(languageModule);
            module.SetAssembly(std::move(assembly));
            module.SetMethodTable(initResult->table);

            return {};
        }

        Result<void> ValidateAndSetPluginData(
            Extension& plugin,
            ILanguageModule* module,
            LoadData& result
        ) {
            auto& [methods, data, table] = result;
            const auto& exportedMethods = plugin.GetMethods();

            if (methods.size() != exportedMethods.size()) {
                return plg::unexpected(std::format(
                    "Method count mismatch: expected {}, got {}",
                    exportedMethods.size(), methods.size()));
            }

            // Validate methods
            std::vector<std::string> errors;
            for (size_t i = 0; i < methods.size(); ++i) {
                const auto& [method, addr] = methods[i];
                const auto& exportedMethod = exportedMethods[i];

                if (&method != &exportedMethod || !addr) {
                    errors.emplace_back(std::format("{:>3}. {}", i + 1, exportedMethod.GetName()));
                    if (constexpr int kMaxDisplay = 10; errors.size() >= kMaxDisplay) {
                        errors.emplace_back(std::format("... and {} more", methods.size() - kMaxDisplay));
                        break;
                    }
                }
            }

            if (!errors.empty()) {
                return plg::unexpected(std::format("Invalid methods:\n{}", plg::join(errors, "\n")));
            }

            plugin.SetLanguageModule(module);
            plugin.SetUserData(data);
            plugin.SetMethodTable(table);
            plugin.SetMethodsData(std::move(methods));

            return {};
        }
    };
}