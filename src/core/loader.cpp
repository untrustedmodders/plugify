#include "loader.hpp"
#include "plugify/asm/load_flag.hpp"
#include "plugify/core/assembly.hpp"
#include "plugify/core/language_module.hpp"
#include "plugify/core/provider.hpp"

using namespace plugify;

#if 0

// void InitiailizeModule(std::shared_ptr<Module> module)
Result<void> Loader::LoadModule(std::shared_ptr<Provider> provider, std::shared_ptr<Module> module) {
    StateTransition scope(module.get(), ModuleState::Unloaded, ModuleState::Loaded);

    LoadFlag flags = LoadFlag::Lazy | LoadFlag::Global | /**/ LoadFlag::SearchUserDirs | LoadFlag::SearchSystem32 | LoadFlag::SearchDllLoadDir;
    if (provider->IsPreferOwnSymbols()) {
        flags |= LoadFlag::Deepbind;
    }

    auto path = provider->GetService<IFileSystem>().Absolute(module->GetRuntime());
    if (!path) {
        return plg::unexpected(path.error());
    }

    auto assemblyResult = provider->GetService<IAssemblyLoader>()->Load(*path, flags);
    if (!assemblyResult) {
        return plg::unexpected(assemblyResult.error());
    }

    auto& assembly = *assemblyResult;
    
	constexpr std::string_view kGetLanguageModuleFn = "GetLanguageModule";

    auto GetLanguageModuleFunc = assembly->GetSymbol(kGetLanguageModuleFn).RCast<ILanguageModule*(*)()>();
    if (!GetLanguageModuleFunc) {
        return plg::unexpected(std::format("Function '{}' not found", kGetLanguageModuleFn));
    }

    auto* languageModule = GetLanguageModuleFunc();
    if (!languageModule) {
        return plg::unexpected(std::format("Returned invalid address from '{}'!", kGetLanguageModuleFn));
    }

#if PLUGIFY_PLATFORM_WINDOWS
    constexpr bool plugifyBuildType = PLUGIFY_IS_DEBUG;
    bool moduleBuildType = languageModule->IsDebugBuild();
    if (moduleBuildType != plugifyBuildType) {
        return plg::unexpected(std::format("Mismatch between plugify ({}) build type and module ({}) build type.", plugifyBuildType ? "debug" : "release", moduleBuildType ? "debug" : "release"));
    }
#endif // PLUGIFY_PLATFORM_WINDOWS

    auto intiResult = languageModule->Initialize(provider, module);
    if (!intiResult) {
        return plg::unexpected(intiResult.error());
    }

    module->SetLanguageModule(languageModule);
    module->SetAssemnly(std::move(assembly));
    module->SetTable(intiResult->table);

    scope.Commit();
    return {};
}
void Loader::UpdateModule(std::shared_ptr<Module> module, double deltaTime) {
    StateTransition scope(module.get(), ModuleState::Loaded, ModuleState::Loaded);

    const auto& [hasUpdate, hasStart, hasEnd, hasExport] = module->GetTable();
    if (hasUpdate) {
        module->GetLanguageModule()->OnUpdate(deltaTime);
    }

    scope.Commit();
}

void Loader::UnloadModule(std::shared_ptr<Module> module) {
    StateTransition scope(module.get(), ModuleState::Loaded, ModuleState::Unloaded);

    if (auto* languageModule = module->GetLanguageModule()) {
        languageModule->Shutdown();
        module->SetLanguageModule(languageModule);
    }
    module->SetAssemnly(nullptr);

    scope.Commit();
}

// void TerminateModule(std::shared_ptr<Module> module)

Result<void> Loader::LoadPlugin(std::shared_ptr<Module> module, std::shared_ptr<Plugin> plugin) {
    StateTransition scope(plugin.get(), PluginState::Unloaded, PluginState::Loaded);

    auto* languageModule = module->GetLanguageModule();
    auto result = languageModule->OnPluginLoad(plugin);
    if (!result) {
        return plg::unexpected(result.error());
    }

    auto& [methods, data, table] = *result;
    const auto& exportedMethods = plugin->GetMethods();
    if (methods.size() != exportedMethods.size()) {
        return plg::unexpected(std::format("Mismatch in methods count, expected: {} but provided: {}", exportedMethods.size(), methods.size()));
    }

    constexpr size_t maxDisplay = 10;
    std::vector<std::string> errors;

    for (size_t i = 0; i < methods.size(); ++i) {
        const auto& [method, addr] = methods[i];
        const auto& exportedMethod = exportedMethods[i];

        if (&method != &exportedMethod || !addr) {
            errors.emplace_back(std::format("{:>2}. {:<15}", i + 1, exportedMethod.GetName()));
        }
        if (maxDisplay > i) {
            errors.emplace_back(std::format(" ... and {} more", methods.size() - maxDisplay));
        }
    }

    if (!errors.empty()) {
        return plg::unexpected(std::format("Invalid method(s):\n{}", plg::join(errors, "\n")));
    }

    plugin->SetTable(table);
    plugin->SetUserData(data);
    plugin->SetMethodsData(std::move(methods));
    plugin->SetModule(std::move(module));
    plugin->SetLanguageModule(languageModule);

    return {};
}

void Loader::StartPlugin(std::shared_ptr<Plugin> plugin) {
    StateTransition scope(plugin.get(), PluginState::Loaded, PluginState::Started);

    const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin->GetTable();
    if (hasStart) {
        plugin->GetLanguageModule()->OnPluginStart(plugin);
    }

    scope.Commit();
}

void Loader::UpdatePlugin(std::shared_ptr<Plugin> plugin, double deltaTime) {
    StateTransition scope(plugin.get(), PluginState::Started, PluginState::Started);

    const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin->GetTable();
    if (hasUpdate) {
        plugin->GetLanguageModule()->OnPluginUpdate(plugin, deltaTime);
    }

    scope.Commit();
}

void Loader::EndPlugin(std::shared_ptr<Plugin> plugin) {
    StateTransition scope(plugin.get(), PluginState::Started, PluginState::Ended);

    const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin->GetTable();
    if (hasEnd) {
        plugin->GetLanguageModule()->OnPluginEnd(plugin);
    }

    scope.Commit();
}

void Loader::UnloadPlugin(std::shared_ptr<Plugin> plugin) {
    StateTransition scope(plugin.get(), PluginState::Ended, PluginState::Unloaded);
    scope.Commit();
}

void Loader::MethodExport(std::shared_ptr<Plugin> plugin) {
    StateTransition scope(plugin.get(), PluginState::Loaded, PluginState::Loaded);

    const auto& [hasUpdate, hasStart, hasEnd, hasExport] = plugin->GetTable();
    if (hasExport) {
       plugin->GetLanguageModule()->OnMethodExport(plugin);
    }

    scope.Commit();
}

#endif
