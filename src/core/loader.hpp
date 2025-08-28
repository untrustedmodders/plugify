#pragma once

#include "plugify/core/assembly.hpp"
#include "plugify/core/language_module.hpp"
#include "plugify/core/module.hpp"
#include "plugify/core/plugin.hpp"
#include "plugify/core/provider.hpp"

namespace plugify {
    struct Loader {
        // void InitiailizeModule(std::shared_ptr<Module> module)
        Result<void> LoadModule(std::shared_ptr<Provider> provider, std::shared_ptr<Module> module);
        void UpdateModule(std::shared_ptr<Module> module, double deltaTime);
        void UnloadModule(std::shared_ptr<Module> module);
        // void TerminateModule(std::shared_ptr<Module> module)
        //void UpdateModules(const std::vector<std::shared_ptr<Module>>& modules, double deltaTime)

        Result<void> LoadPlugin(std::shared_ptr<Module> module, std::shared_ptr<Plugin> plugin);
        void StartPlugin(std::shared_ptr<Plugin> plugin);
        void UpdatePlugin(std::shared_ptr<Plugin> plugin, double deltaTime);
        void EndPlugin(std::shared_ptr<Plugin> plugin);
        void UnloadPlugin(std::shared_ptr<Plugin> plugin);
        //void UpdatePlugins(const std::vector<std::shared_ptr<Plugin>>& plugins, double deltaTime);

        void MethodExport(std::shared_ptr<Plugin> plugin);
    };

    template<typename T, typename S>
    struct StateTransition {
        T impl;
        S expected;
        S next;
        bool committed = false;

        StateTransition(T i, S exp, S nxt)
            : impl(i), expected(exp), next(nxt)
        {
            PL_ASSERT(impl->GetState() == expected && "Invalid state transition");
        }

        void Rollback(S errorState) {
            impl->SetState(errorState);
            committed = true;
        }

        void Commit() {
            impl->SetState(next);
            committed = true;
        }

        ~StateTransition() {
            if (!committed) {
                // Fails loudly if you forgot to commit
                PL_ASSERT(false && "State transition not committed");
            }
        }
    };

    // Advanced: Exception-safe RAII wrapper for plugin calls
    template<typename Func>
    class SafeCall {
        std::string_view operation;
        std::string_view packageName;

    public:
        SafeCall(std::string_view op, std::string_view name)
            : operation(op), packageName(name) {}

        Result<void> Execute(Func&& func) noexcept {
            try {
                return func();
            } catch (const std::bad_alloc&) {
                return plg::unexpected(std::format("{}: out of memory", operation));
            } catch (const std::exception& e) {
                return plg::unexpected(std::format("{} failed for '{}': {}", operation, packageName, e.what()));
            } catch (...) {
                return plg::unexpected(std::format("{} failed for '{}': unknown exception",  operation, packageName));
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
            std::string slowestPackage;
        };

    private:
        LoadStatistics stats;

        // Optional: Cache for preloaded binaries
        //std::unordered_map<std::filesystem::path, std::shared_ptr<IAssembly>, plg::path_hash> assemblyCache;

    public:
        // Module Operations
        Result<void> LoadModule(std::shared_ptr<Provider> provider, std::shared_ptr<Module> module) {
            auto timer = ScopedTimer([&](Duration elapsed) {
                stats.totalLoadTime += elapsed;
                if (elapsed > stats.slowestLoad) {
                    stats.slowestLoad = elapsed;
                    stats.slowestPackage = module->GetName();
                }
            });

            StateTransition scope(module.get(), ModuleState::Unloaded, ModuleState::Loaded);

            // Try to use preloaded assembly if available
            auto assemblyResult = GetOrLoadAssembly(provider, module->GetRuntime(), module->GetDirectories());
            if (!assemblyResult) {
                return plg::unexpected(assemblyResult.error());
            }

            // Load language module interface
            auto langModuleResult = LoadLanguageModule(*assemblyResult, provider, module);
            if (!langModuleResult) {
                // Rollback on failure
                scope.Rollback(ModuleState::Failed);
                return langModuleResult;
            }

            stats.modulesLoaded++;
            scope.Commit();
            return {};
        }

        void UnloadModule(std::shared_ptr<Module> module) {
            StateTransition scope(module.get(), ModuleState::Loaded, ModuleState::Unloaded);

            if (auto* languageModule = module->GetLanguageModule()) {
                languageModule->Shutdown();
                module->SetLanguageModule(nullptr);
            }

            // Clear assembly and remove from cache
            if (auto assembly = module->GetAssembly()) {
                //assemblyCache.erase(module->GetRuntime());
                module->SetAssembly(nullptr);
            }

            scope.Commit();
        }

        // Plugin Operations
        Result<void> LoadPlugin(std::shared_ptr<Module> module,
                              std::shared_ptr<Plugin> plugin) {
            auto timer = ScopedTimer([&](Duration elapsed) {
                stats.totalLoadTime += elapsed;
                if (elapsed > stats.slowestLoad) {
                    stats.slowestLoad = elapsed;
                    stats.slowestPackage = plugin->GetName();
                }
            });

            StateTransition scope(plugin.get(), PluginState::Unloaded, PluginState::Loaded);

            // Validate module is loaded
            if (!module || module->GetState() != ModuleState::Loaded) {
                return plg::unexpected("Module not loaded");
            }

            auto* languageModule = module->GetLanguageModule();
            if (!languageModule) {
                return plg::unexpected("Language module not available");
            }

            // Load plugin through language module
            auto result = languageModule->OnPluginLoad(plugin);
            if (!result) {
                scope.Rollback(PluginState::Failed);
                return plg::unexpected(result.error());
            }

            // Validate and set plugin data
            auto validateResult = ValidateAndSetPluginData(plugin, module, *result);
            if (!validateResult) {
                scope.Rollback(PluginState::Failed);
                return validateResult;
            }

            stats.pluginsLoaded++;
            scope.Commit();
            return {};
        }

        Result<void> StartPlugin(std::shared_ptr<Plugin> plugin) {
            StateTransition scope(plugin.get(), PluginState::Loaded, PluginState::Started);

            const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin->GetTable();
            if (hasStart) {
                plugin->GetLanguageModule()->OnPluginStart(plugin);
            }

            scope.Commit();
            return {};
        }

        Result<void> EndPlugin(std::shared_ptr<Plugin> plugin) {
            StateTransition scope(plugin.get(), PluginState::Started, PluginState::Ended);

            const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin->GetTable();
            if (hasEnd) {
                plugin->GetLanguageModule()->OnPluginEnd(plugin);
            }

            scope.Commit();
            return {};
        }

        void UnloadPlugin(std::shared_ptr<Plugin> plugin) {
            StateTransition scope(plugin.get(), PluginState::Ended, PluginState::Unloaded);

            // Clear all plugin data
            plugin->SetModule(nullptr);
            plugin->SetLanguageModule(nullptr);
            plugin->SetUserData(nullptr);
            plugin->SetMethodsData({});

            scope.Commit();
        }

        // Batch operations for efficiency
        void UpdateModules(const std::vector<std::shared_ptr<Module>>& modules, double deltaTime) {
            for (auto& module : modules) {
                if (module->GetState() != ModuleState::Loaded) continue;

                const auto& [hasUpdate, hasStart, hasEnd, hasExport] = module->GetTable();
                if (hasUpdate) {
                    module->GetLanguageModule()->OnUpdate(deltaTime);
                }
            }
        }

        void UpdatePlugins(const std::vector<std::shared_ptr<Plugin>>& plugins, double deltaTime) {
            for (auto& plugin : plugins) {
                if (plugin->GetState() != PluginState::Started) continue;

                const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin->GetTable();
                if (hasUpdate) {
                    plugin->GetLanguageModule()->OnPluginUpdate(plugin, deltaTime);
                }
            }
        }

        // Preload support
        /*Result<void> PreloadAssembly(std::shared_ptr<Provider> provider,
                                    const std::filesystem::path& path) {
            auto absPath = provider->GetService<IFileSystem>().Absolute(path);
            if (!absPath) {
                return plg::unexpected(absPath.error());
            }

            // Check if already cached
            if (assemblyCache.contains(*absPath)) {
                return {};
            }

            LoadFlag flags = GetPreLoadFlags(provider);
            auto assemblyResult = provider->GetService<IAssemblyLoader>()->Load(*absPath, flags);
            if (!assemblyResult) {
                return plg::unexpected(assemblyResult.error());
            }

            assemblyCache[*absPath] = *assemblyResult;
            return {};
        }*/

        // Statistics
        const LoadStatistics& GetStatistics() const { return stats; }
        void ResetStatistics() { stats = {}; }

    private:
        // Helper to get or load assembly
        Result<std::shared_ptr<IAssembly>> GetOrLoadAssembly(
            std::shared_ptr<Provider> provider,
            const std::filesystem::path& path,
            std::vector<std::filesystem::path> searchPaths
        ) {
            auto absPath = provider->GetService<IFileSystem>().Absolute(path);
            if (!absPath) {
                return plg::unexpected(absPath.error());
            }

            // Check cache first
            /*if (auto it = assemblyCache.find(*absPath); it != assemblyCache.end()) {
                return it->second;
            }*/

            for (const auto& searchPath : searchPaths) {
                provider->GetService<IAssemblyLoader>()->AddSearchPath(searchPath);
            }

            // Load assembly
            LoadFlag flags = GetLoadFlags(provider);
            auto assemblyResult = provider->GetService<IAssemblyLoader>()->Load(*absPath, flags);
            if (!assemblyResult) {
                return plg::unexpected(assemblyResult.error());
            }

            // Cache for future use
            //assemblyCache[*absPath] = *assemblyResult;
            return *assemblyResult;
        }

        static LoadFlag GetLoadFlags(std::shared_ptr<Provider> provider) {
            LoadFlag flags = LoadFlag::Lazy | LoadFlag::Global |
                            LoadFlag::SearchUserDirs | LoadFlag::SearchSystem32 |
                            LoadFlag::SearchDllLoadDir;

            if (provider->IsPreferOwnSymbols()) {
                flags |= LoadFlag::Deepbind;
            }

            return flags;
        }

        static LoadFlag GetPreLoadFlags() {
            LoadFlag flags = LoadFlag::Lazy | LoadFlag::DontResolveDllReferences;
            return flags;
        }

        Result<void> LoadLanguageModule(
            std::shared_ptr<IAssembly> assembly,
            std::shared_ptr<Provider> provider,
            std::shared_ptr<Module> module)
        {
            constexpr std::string_view kGetLanguageModuleFn = "GetLanguageModule";

            auto GetLanguageModuleFunc = assembly->GetSymbol(kGetLanguageModuleFn)
                .RCast<ILanguageModule*(*)()>();
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

            auto initResult = languageModule->Initialize(provider, module);
            if (!initResult) {
                return plg::unexpected(initResult.error());
            }

            module->SetLanguageModule(languageModule);
            module->SetAssembly(std::move(assembly));
            module->SetTable(initResult->table);

            return {};
        }

        Result<void> ValidateAndSetPluginData(
            std::shared_ptr<Plugin> plugin,
            std::shared_ptr<Module> module,
            Result<LoadData>& result
        ) {
            auto& [methods, data, table] = result;
            const auto& exportedMethods = plugin->GetMethods();

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
                    if (errors.size() >= 10) {
                        errors.emplace_back(std::format("... and {} more", methods.size() - 10));
                        break;
                    }
                }
            }

            if (!errors.empty()) {
                return plg::unexpected(std::format("Invalid methods:\n{}", plg::join(errors, "\n")));
            }

            plugin->SetTable(table);
            plugin->SetUserData(data);
            plugin->SetMethodsData(std::move(methods));
            plugin->SetLanguageModule(module->GetLanguageModule());
            plugin->SetModule(std::move(module));

            return {};
        }
    };
}