#pragma once

#include "plugify/assembly.hpp"
#include "plugify/config.hpp"
#include "plugify/extension.hpp"
#include "plugify/file_system.hpp"
#include "plugify/language_module.hpp"
#include "plugify/provider.hpp"
#include "plugify/lifecycle.hpp"
#include "plugify/registrar.hpp"

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
                return MakeError("{}: out of memory", operation);
            } catch (const std::exception& e) {
                return MakeError("{} failed for '{}': {}", operation, extensionName, e.what());
            } catch (...) {
                return MakeError("{} failed for '{}': unknown exception",  operation, extensionName);
            }
        }
    };

    // Enhanced Loader class with better error handling and preloading support
    class ExtensionLoader {
    public:
        struct LoadStatistics {
            size_t modulesLoaded{0};
            size_t pluginsLoaded{0};
            Duration totalLoadTime{};

            Duration slowestModuleLoad{};
            UniqueId slowestModule;
            Duration slowestPluginLoad{};
            UniqueId slowestPlugin;

            std::string ToString() const {
                return std::format(
                    "=== Loader Report ===\n"
                    "  Modules loaded: {}\n"
                    "  Plugins loaded: {}\n"
                    "  Slowest module: {} - {}\n"
                    "  Slowest plugin: {} - {}\n"
                    "  Total load time: {}\n",
                    modulesLoaded,
                    pluginsLoaded,
                    slowestModuleLoad,
                    ToShortString(slowestModule),
                    slowestPluginLoad,
                    ToShortString(slowestPlugin),
                    totalLoadTime
                );
            }
        };

    private:
        const Config& _config;
        const Provider& _provider;
        std::shared_ptr<IFileSystem> _fileSystem;
        std::shared_ptr<IAssemblyLoader> _assemblyLoader;
        std::shared_ptr<ILifecycle> _lifecycle;
        LoadStatistics _stats;

        std::unordered_map<std::filesystem::path, std::shared_ptr<IAssembly>, plg::path_hash> _assemblyCache;

    public:
        ExtensionLoader(
            const ServiceLocator& locator,
            const Config& config,
            const Provider& provider
        )
            : _config(config)
            , _provider(provider)
            , _fileSystem(locator.Resolve<IFileSystem>())
            , _assemblyLoader(locator.Resolve<IAssemblyLoader>())
            , _lifecycle(locator.Resolve<ILifecycle>())
        {}

        // Module Operations
        Result<void> LoadModule(Extension& module) {
            auto timer = ScopedTimer([&](Duration elapsed) {
                _stats.totalLoadTime += elapsed;
                if (elapsed > _stats.slowestModuleLoad) {
                    _stats.slowestModuleLoad = elapsed;
                    _stats.slowestModule = module.GetId();
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

            _lifecycle->OnLoad(module);
            ++_stats.modulesLoaded;
            return {};
        }

        Result<void> UpdateModule(Extension& module, Duration deltaTime) {
            const auto& [hasUpdate, hasStart, hasEnd, hasExport] = module.GetMethodTable();
            if (!hasUpdate) {
                return {};
            }
            auto result = SafeCall("OnUpdate", module.GetName()).Execute<void>([&] {
                module.GetLanguageModule()->OnUpdate(deltaTime);
                return Result<void>{};
            });
            _lifecycle->OnUpdate(module, deltaTime);
            return result;
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
                _assemblyCache.erase(module.GetRuntime());
                module.SetAssembly(nullptr);
            }

            _lifecycle->OnUnload(module);
            --_stats.modulesLoaded;
            return result;
        }

        // Plugin Operations
        Result<void> LoadPlugin(const Extension& module, Extension& plugin) {
            auto timer = ScopedTimer([&](Duration elapsed) {
                _stats.totalLoadTime += elapsed;
                if (elapsed > _stats.slowestPluginLoad) {
                    _stats.slowestPluginLoad = elapsed;
                    _stats.slowestPlugin = plugin.GetId();
                }
            });

            auto* languageModule = module.GetLanguageModule();
            if (!languageModule) {
                return plg::unexpected("Language module not available");
            }

            plugin.SetLanguageModule(languageModule);

            // Load plugin through language module
            auto loadResult = SafeCall("OnPluginLoad", plugin.GetName()).Execute<LoadData>([&] {
                return plugin.GetLanguageModule()->OnPluginLoad(plugin);
            });
            if (!loadResult) {
                return plg::unexpected(std::move(loadResult.error()));
            }

            // Validate and set plugin data
            auto validateResult = ValidateAndSetPluginData(plugin, *loadResult);
            if (!validateResult) {
                return validateResult;
            }

            ++_stats.pluginsLoaded;
            _lifecycle->OnLoad(plugin);
            return {};
        }

        Result<void> StartPlugin(Extension& plugin) {
            const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin.GetMethodTable();
            if (!hasStart) {
                return {};
            }
            auto result = SafeCall("OnPluginStart", plugin.GetName()).Execute<void>([&] {
                plugin.GetLanguageModule()->OnPluginStart(plugin);
                return Result<void>{};
            });
            _lifecycle->OnStart(plugin);
            return result;
        }

        Result<void> EndPlugin(Extension& plugin) {
            const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin.GetMethodTable();
            if (!hasEnd) {
                return {};
            }
            auto result = SafeCall("OnPluginEnd", plugin.GetName()).Execute<void>([&] {
                plugin.GetLanguageModule()->OnPluginEnd(plugin);
                return Result<void>{};
            });
            _lifecycle->OnEnd(plugin);
            return result;
        }

        Result<void> UpdatePlugin(Extension& plugin, Duration deltaTime) {
            const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin.GetMethodTable();
            if (!hasUpdate) {
                return {};
            }
            auto result = SafeCall("OnPluginUpdate", plugin.GetName()).Execute<void>([&] {
                plugin.GetLanguageModule()->OnPluginUpdate(plugin, deltaTime);
                return Result<void>{};
            });
            _lifecycle->OnUpdate(plugin, deltaTime);
            return result;
        }

        Result<void> UnloadPlugin(Extension& plugin) {
            // Clear all plugin data
            plugin.SetLanguageModule(nullptr);
            plugin.SetUserData(nullptr);
            plugin.SetMethodTable({});
            plugin.SetMethodsData({});
            _lifecycle->OnUnload(plugin);
            --_stats.pluginsLoaded;
            return {};
        }

        Result<void> MethodExport(const Extension& module, const Extension& plugin) {
            const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin.GetMethodTable();
            if (!hasExport) {
                return {};
            }
            auto result = SafeCall("OnMethodExport", module.GetName()).Execute<void>([&] {
                module.GetLanguageModule()->OnMethodExport(plugin);
                return Result<void>{};
            });
            //_lifecycle->OnExport(module, plugin);
            return result;
        }

        // Preload support
        /*Result<void> PreloadAssembly(
            const std::filesystem::path& path,
            const std::vector<std::filesystem::path>& searchPaths
        ) {
            auto absPath = _fileSystem->GetAbsolutePath(path);
            if (!absPath) {
                return plg::unexpected(absPath.error());
            }

            // Check if already cached
            if (_assemblyCache.contains(*absPath)) {
                return {};
            }

            LoadFlag flags = GetLoadFlags();
            auto assemblyResult = _assemblyLoader->Load(*absPath, flags, searchPaths);
            if (!assemblyResult) {
                return plg::unexpected(assemblyResult.error());
            }

            _assemblyCache.emplace(std::move(*absPath), std::move(*assemblyResult));
            return {};
        }*/

        // Statistics
        const LoadStatistics& GetStatistics() const { return _stats; }
        void ResetStatistics() { _stats = {}; }

        void Clear() { _assemblyCache.clear(); }

    private:
        // Helper to get or load assembly
        Result<std::shared_ptr<IAssembly>> GetOrLoadAssembly(
            const std::filesystem::path& path,
            const std::vector<std::filesystem::path>& searchPaths
        ) {
            auto absPath = _fileSystem->GetAbsolutePath(path);
            if (!absPath) {
                return plg::unexpected(std::move(absPath.error()));
            }

            // Check cache first
            if (auto it = _assemblyCache.find(*absPath); it != _assemblyCache.end()) {
                return it->second;
            }

            // Load assembly
            LoadFlag flags = GetLoadFlags();
            auto assemblyResult = _assemblyLoader->Load(*absPath, flags, searchPaths);
            if (!assemblyResult) {
                return plg::unexpected(std::move(assemblyResult.error()));
            }

            // Cache for future use
            _assemblyCache.emplace(std::move(*absPath), *assemblyResult);
            return std::move(*assemblyResult);
        }

        LoadFlag GetLoadFlags() const {
            LoadFlag flags = LoadFlag::LazyBinding | LoadFlag::GlobalSymbols | LoadFlag::SecureSearch;
            if (_config.loading.preferOwnSymbols) {
                flags |= LoadFlag::DeepBind;
            }
            return flags;
        }

        Result<void> LoadLanguageModule(
            std::shared_ptr<IAssembly> assembly,
            Extension& module)
        {
            constexpr std::string_view kGetLanguageModuleFn = "GetLanguageModule";

            auto entryFunc = assembly->GetSymbol(kGetLanguageModuleFn);
            if (!entryFunc) {
                return plg::unexpected(std::move(entryFunc.error()));
            }

            auto* languageModule = entryFunc->RCast<ILanguageModule*(*)()>()();
            if (!languageModule) {
                return MakeError("Invalid address from '{}'", kGetLanguageModuleFn);
            }

    #if PLUGIFY_PLATFORM_WINDOWS
            constexpr bool plugifyBuildType = PLUGIFY_IS_DEBUG;
            bool moduleBuildType = languageModule->IsDebugBuild();
            if (moduleBuildType != plugifyBuildType) {
                return MakeError(
                    "Build type mismatch: plugify={}, module={}",
                    plugifyBuildType ? "debug" : "release",
                    moduleBuildType ? "debug" : "release");
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
            LoadData& result
        ) {
            auto& [methods, data, table] = result;
            const auto& exportedMethods = plugin.GetMethods();

            if (methods.size() != exportedMethods.size()) {
                return MakeError(
                    "Method count mismatch: expected {}, got {}",
                    exportedMethods.size(), methods.size());
            }

            // Validate methods
            std::vector<std::string> errors;
            for (size_t i = 0; i < methods.size(); ++i) {
                const auto& [method, addr] = methods[i];
                const auto& exportedMethod = exportedMethods[i];

                if (&method != &exportedMethod || !addr) {
                    errors.push_back(std::format("{:>3}. {}", i + 1, exportedMethod.GetName()));
                    if (constexpr size_t kMaxDisplay = 10; errors.size() >= kMaxDisplay) {
                        errors.push_back(std::format("... and {} more", methods.size() - kMaxDisplay));
                        break;
                    }
                }
            }

            if (!errors.empty()) {
                return MakeError("Invalid methods:\n{}", plg::join(errors, "\n"));
            }

            plugin.SetUserData(data);
            plugin.SetMethodTable(table);
            plugin.SetMethodsData(std::move(methods));

            return {};
        }
    };
}