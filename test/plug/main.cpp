#include "plugify/core/plugify.hpp"
#include "plugify/core/manager.hpp"
#include "plugify/core/plugify.hpp"
#include "plugify/core/manager.hpp"
#include "plugify/core/extension.hpp"
#include "plugify/core/logger.hpp"

#include <CLI/CLI.hpp>
#include <plg/format.hpp>

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

#define PLG_COUT(x) std::cout << x << std::endl
#define PLG_CERR(x) std::cerr << x << std::endl
#define PLG_COUT_FMT(...) std::cout << std::format(__VA_ARGS__) << std::endl
#define PLG_CERR_FMT(...) std::cerr << std::format(__VA_ARGS__) << std::endl

namespace {
    plugify::UniqueId FormatId(std::string_view str) {
        plugify::UniqueId result;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);

        if (ec != std::errc{}) {
            PLG_CERR_FMT("Error: {}", std::make_error_code(ec).message());
            return -1;
        } else if (ptr != str.data() + str.size()) {
            PLG_CERR("Invalid argument: trailing characters after the valid part");
            return -1;
        }

        return result;
    }

    // Helper to format duration
    std::string FormatDuration(plugify::Duration duration) {
        using namespace std::chrono;

        auto ns = duration_cast<nanoseconds>(duration).count();
        if (ns < 1000) {
            return std::format("{}ns", ns);
        } else if (ns < 1000000) {
            return std::format("{:.2f}μs", ns / 1000.0);
        } else if (ns < 1000000000) {
            return std::format("{:.2f}ms", ns / 1000000.0);
        } else {
            return std::format("{:.2f}s", ns / 1000000000.0);
        }
    }

    // Helper to get state color/symbol
    std::string GetStateIndicator(plugify::ExtensionState state) {
        using namespace plugify;
        switch (state) {
            case ExtensionState::Started:
            case ExtensionState::Loaded:
                return "v"; // or "[OK]" if unicode not supported
            case ExtensionState::Failed:
            case ExtensionState::Corrupted:
            case ExtensionState::Unresolved:
                return "x"; // or "[FAIL]"
            case ExtensionState::Disabled:
            case ExtensionState::Skipped:
                return "¤"; // or "[SKIP]"
            case ExtensionState::Loading:
            case ExtensionState::Starting:
            case ExtensionState::Parsing:
            case ExtensionState::Resolving:
                return "⋯"; // or "[...]"
            default:
                return "?"; // or "[?]"
        }
    }

    // Helper to truncate string with ellipsis
    std::string Truncate(std::string str, size_t maxLen) {
        if (str.length() <= maxLen) return str;
        return str.substr(0, maxLen - 3) + "...";
    }

    // Helper to format file size
    std::string FormatFileSize(const std::filesystem::path& path) {
        try {
            auto size = std::filesystem::file_size(path);
            if (size < 1024) {
                return std::format("{} B", size);
            } else if (size < 1024 * 1024) {
                return std::format("{:.1f} KB", static_cast<double>(size) / 1024.0);
            } else if (size < 1024 * 1024 * 1024) {
                return std::format("{:.1f} MB", static_cast<double>(size) / (1024.0 * 1024.0));
            } else {
                return std::format("{:.1f} GB", static_cast<double>(size) / (1024.0 * 1024.0 * 1024.0));
            }
        } catch (...) {
            return "N/A";
        }
    }
}

using namespace plugify;

class PlugifyApp {
public:
    PlugifyApp(std::shared_ptr<Plugify> plugify) : plug(std::move(plugify)) {}

    void Initialize() {
        if (!plug->Initialize()) {
            PLG_CERR("No feet, no sweets!");
            throw std::runtime_error("Failed to initialize Plugify");
        }
        plug->GetManager().Initialize();
        PLG_COUT("Plugin system initialized.");
    }

    void Terminate() {
        plug->Terminate();
        PLG_COUT("Plugin system terminated.");
    }

    void ShowVersion() {
        static std::string copyright = std::format("Copyright (C) 2023-{}{}{}{} Untrusted Modders Team", 
                                                   __DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10]);
        PLG_COUT(R"(      ____)");
        PLG_COUT(R"( ____|    \         Plugify )" << plug->GetVersion().to_string());
        PLG_COUT(R"((____|     `._____  )" << copyright);
        PLG_COUT(R"( ____|       _|___)");
        PLG_COUT(R"((____|     .'       This program may be freely redistributed under)");
        PLG_COUT(R"(     |____/         the terms of the MIT License.)");
    }

    void LoadManager() {
        if (!plug->IsInitialized()) {
            PLG_CERR("Initialize system before use.");
            return;
        }
        const auto& manager = plug->GetManager();
        if (manager.IsInitialized()) {
            PLG_CERR("Plugin manager already loaded.");
        } else {
            manager.Initialize();
            PLG_COUT("Plugin manager was loaded.");
        }
    }

    void UnloadManager() {
        if (!plug->IsInitialized()) {
            PLG_CERR("Initialize system before use.");
            return;
        }
        const auto& manager = plug->GetManager();
        if (!manager.IsInitialized()) {
            PLG_CERR("Plugin manager already unloaded.");
        } else {
            manager.Terminate();
            PLG_COUT("Plugin manager was unloaded.");
        }
    }

    void ReloadManager() {
        if (!plug->IsInitialized()) {
            PLG_CERR("Initialize system before use.");
            return;
        }
        const auto& manager = plug->GetManager();
        if (!manager.IsInitialized()) {
            PLG_CERR("Plugin manager not loaded.");
        } else {
            manager.Terminate();
            manager.Initialize();
            PLG_COUT("Plugin manager was reloaded.");
        }
    }

#if 0
    void ListPlugins() {
        if (!CheckManager()) return;

        const auto& manager = plug->GetManager();
        auto plugins = manager.GetExtensionsByType(ExtensionType::Plugin);
        auto count = plugins.size();

        if (!count) {
            PLG_CERR("No plugins loaded.");
            return;
        }

        PLG_COUT_FMT("Listing {} plugin{}:", count, (count > 1) ? "s" : "");
        PLG_COUT(std::string(80, '-'));

        // Header
        PLG_COUT_FMT("{:<3} {:<25} {:<15} {:<12} {:<8} {:<12}",
                     "#", "Name", "Version", "State", "Lang", "Load Time");
        PLG_COUT(std::string(80, '-'));

        size_t index = 1;
        for (const auto& plugin : plugins) {
            auto state = plugin->GetState();
            auto stateStr = plg::enum_to_string(state);
            auto indicator = GetStateIndicator(state);

            // Get load time if available
            std::string loadTime = "N/A";
            try {
                auto duration = plugin->GetOperationTime(ExtensionState::Loaded);
                if (duration.count() > 0) {
                    loadTime = FormatDuration(duration);
                }
            } catch (...) {}

            PLG_COUT_FMT("{:<3} {:<25} {:<15} {} {:<11} {:<8} {:<12}",
                         index++,
                         Truncate(plugin->GetName(), 24),
                         plugin->GetVersionString(),
                         indicator,
                         Truncate(std::string(stateStr), 10),
                         Truncate(plugin->GetLanguage(), 7),
                         loadTime);

            // Show errors/warnings if any
            if (plugin->HasErrors()) {
                for (const auto& error : plugin->GetErrors()) {
                    PLG_CERR_FMT("     └─ Error: {}", error);
                }
            }
            for (const auto& warning : plugin->GetWarnings()) {
                PLG_COUT_FMT("     └─ Warning: {}", warning);
            }
        }
        PLG_COUT(std::string(80, '-'));
    }

    void ListModules() {
        if (!CheckManager()) return;

        const auto& manager = plug->GetManager();
        auto modules = manager.GetExtensionsByType(ExtensionType::Module);
        auto count = modules.size();

        if (!count) {
            PLG_CERR("No modules loaded.");
            return;
        }

        PLG_COUT_FMT("Listing {} module{}:", count, (count > 1) ? "s" : "");
        PLG_COUT(std::string(80, '-'));

        // Header
        PLG_COUT_FMT(
            "{:<3} {:<25} {:<15} {:<12} {:<8} {:<12}",
            "#",
            "Name",
            "Version",
            "State",
            "Lang",
            "Load Time"
        );
        PLG_COUT(std::string(80, '-'));

        size_t index = 1;
        for (const auto& module : modules) {
            auto state = module->GetState();
            auto stateStr = plg::enum_to_string(state);
            auto indicator = GetStateIndicator(state);

            // Get load time if available
            std::string loadTime = "N/A";
            try {
                auto duration = module->GetOperationTime(ExtensionState::Loaded);
                if (duration.count() > 0) {
                    loadTime = FormatDuration(duration);
                }
            } catch (...) {
            }

            PLG_COUT_FMT(
                "{:<3} {:<25} {:<15} {} {:<11} {:<8} {:<12}",
                index++,
                Truncate(module->GetName(), 24),
                module->GetVersionString(),
                indicator,
                Truncate(std::string(stateStr), 10),
                Truncate(module->GetLanguage(), 7),
                loadTime
            );

            // Show errors/warnings if any
            if (module->HasErrors()) {
                for (const auto& error : module->GetErrors()) {
                    PLG_CERR_FMT("     └─ Error: {}", error);
                }
            }
            for (const auto& warning : module->GetWarnings()) {
                PLG_COUT_FMT("     └─ Warning: {}", warning);
            }
        }
        PLG_COUT(std::string(80, '-'));
    }

    void ShowPlugin(std::string_view name, bool useId) {
        if (!CheckManager()) return;

        const auto& manager = plug->GetManager();
        auto plugin = useId ? manager.FindExtension(FormatId(name)) : manager.FindExtension(name);

        if (!plugin) {
            PLG_CERR_FMT("Plugin {} not found.", name);
            return;
        }

        if (!plugin->IsPlugin()) {
            PLG_CERR_FMT("'{}' is not a plugin (it's a module).", name);
            return;
        }

        // Display detailed plugin information
        PLG_COUT(std::string(80, '='));
        PLG_COUT_FMT("PLUGIN INFORMATION: {}", plugin->GetName());
        PLG_COUT(std::string(80, '='));

        // Basic Information
        PLG_COUT("\n[Basic Information]");
        PLG_COUT_FMT("  ID:          {}", plugin->GetId());
        PLG_COUT_FMT("  Name:        {}", plugin->GetName());
        PLG_COUT_FMT("  Version:     {}", plugin->GetVersionString());
        PLG_COUT_FMT("  Language:    {}", plugin->GetLanguage());
        PLG_COUT_FMT("  State:       {} {}", GetStateIndicator(plugin->GetState()),
                     plg::enum_to_string(plugin->GetState()));
        PLG_COUT_FMT("  Location:    {}", plugin->GetLocation().string());
        PLG_COUT_FMT("  File Size:   {}", FormatFileSize(plugin->GetLocation()));

        // Optional Information
        if (!plugin->GetDescription().empty() || !plugin->GetAuthor().empty() ||
            !plugin->GetWebsite().empty() || !plugin->GetLicense().empty()) {
            PLG_COUT("\n[Additional Information]");
            if (!plugin->GetDescription().empty())
                PLG_COUT_FMT("  Description: {}", plugin->GetDescription());
            if (!plugin->GetAuthor().empty())
                PLG_COUT_FMT("  Author:      {}", plugin->GetAuthor());
            if (!plugin->GetWebsite().empty())
                PLG_COUT_FMT("  Website:     {}", plugin->GetWebsite());
            if (!plugin->GetLicense().empty())
                PLG_COUT_FMT("  License:     {}", plugin->GetLicense());
        }

        // Plugin-specific information
        if (!plugin->GetEntry().empty()) {
            PLG_COUT("\n[Plugin Details]");
            PLG_COUT_FMT("  Entry Point: {}", plugin->GetEntry());
        }

        // Methods
        const auto& methods = plugin->GetMethods();
        if (!methods.empty()) {
            PLG_COUT_FMT("\n[Exported Methods] ({} total)", methods.size());
            for (size_t i = 0; i < std::min(methods.size(), size_t(10)); ++i) {
                const auto& method = methods[i];
                PLG_COUT_FMT("  #{:<2} {}", i + 1, method.GetName());
                if (!method.funcType.empty()) {
                    PLG_COUT_FMT("      Type: {}", method.funcType);
                }
            }
            if (methods.size() > 10) {
                PLG_COUT_FMT("  ... and {} more methods", methods.size() - 10);
            }
        }

        // Platforms
        const auto& platforms = plugin->GetPlatforms();
        if (!platforms.empty()) {
            PLG_COUT("\n[Supported Platforms]");
            PLG_COUT("  " + plg::join(platforms, ", "));
        }

        // Dependencies
        const auto& deps = plugin->GetDependencies();
        if (!deps.empty()) {
            PLG_COUT_FMT("\n[Dependencies] ({} total)", deps.size());
            for (const auto& dep : deps) {
                PLG_COUT_FMT("  - {} ({})", dep.GetName(), dep.GetConstraints());
                if (dep.IsOptional()) {
                    PLG_COUT("    └─ Optional");
                }
            }
        }

        // Conflicts
        const auto& conflicts = plugin->GetConflicts();
        if (!conflicts.empty()) {
            PLG_COUT_FMT("\n[Conflicts] ({} total)", conflicts.size());
            for (const auto& conflict : conflicts) {
                PLG_COUT_FMT("  - {} ({})", conflict.GetName(), conflict.GetConstraints());
                const auto& reason = conflict.GetReason();
                if (!reason.empty()) {
                    PLG_COUT_FMT("    └─ Reason: {}", reason);
                }
            }
        }

        // Performance Information
        PLG_COUT("\n[Performance Metrics]");
        PLG_COUT_FMT("  Total Time:  {}", FormatDuration(plugin->GetTotalTime()));

        // Show timing for different operations
        ExtensionState operations[] = {
            ExtensionState::Parsing,
            ExtensionState::Resolving,
            ExtensionState::Loading,
            ExtensionState::Starting
        };

        for (const auto& op : operations) {
            try {
                if (auto duration = plugin->GetOperationTime(op); duration.count() > 0) {
                    PLG_COUT_FMT("  {:<12} {}", plg::enum_to_string(op) + ":"s, FormatDuration(duration));
                }
            } catch (...) {}
        }

        // Errors and Warnings
        if (plugin->HasErrors() || !plugin->GetWarnings().empty()) {
            PLG_COUT("\n[Issues]");
            for (const auto& error : plugin->GetErrors()) {
                PLG_CERR_FMT("  ERROR: {}", error);
            }
            for (const auto& warning : plugin->GetWarnings()) {
                PLG_COUT_FMT("  WARNING: {}", warning);
            }
        }

        PLG_COUT(std::string(80, '='));
    }

    void ShowModule(std::string_view name, bool useId) {
        if (!CheckManager()) return;

        const auto& manager = plug->GetManager();
        auto module = useId ? manager.FindExtension(FormatId(name)) : manager.FindExtension(name);

        if (!module) {
            PLG_CERR_FMT("Module {} not found.", name);
            return;
        }

        if (!module->IsModule()) {
            PLG_CERR_FMT("'{}' is not a module (it's a plugin).", name);
            return;
        }

        // Display detailed module information
        PLG_COUT(std::string(80, '='));
        PLG_COUT_FMT("MODULE INFORMATION: {}", module->GetName());
        PLG_COUT(std::string(80, '='));

        // Basic Information
        PLG_COUT("\n[Basic Information]");
        PLG_COUT_FMT("  ID:          {}", module->GetId());
        PLG_COUT_FMT("  Name:        {}", module->GetName());
        PLG_COUT_FMT("  Version:     {}", module->GetVersionString());
        PLG_COUT_FMT("  Language:    {}", module->GetLanguage());
        PLG_COUT_FMT("  State:       {} {}", GetStateIndicator(module->GetState()),
                     plg::enum_to_string(module->GetState()));
        PLG_COUT_FMT("  Location:    {}", module->GetLocation().string());
        PLG_COUT_FMT("  File Size:   {}", FormatFileSize(module->GetLocation()));

        // Optional Information
        if (!module->GetDescription().empty() || !module->GetAuthor().empty() ||
            !module->GetWebsite().empty() || !module->GetLicense().empty()) {
            PLG_COUT("\n[Additional Information]");
            if (!module->GetDescription().empty())
                PLG_COUT_FMT("  Description: {}", module->GetDescription());
            if (!module->GetAuthor().empty())
                PLG_COUT_FMT("  Author:      {}", module->GetAuthor());
            if (!module->GetWebsite().empty())
                PLG_COUT_FMT("  Website:     {}", module->GetWebsite());
            if (!module->GetLicense().empty())
                PLG_COUT_FMT("  License:     {}", module->GetLicense());
        }

        // Module-specific information
        if (!module->GetRuntime().empty()) {
            PLG_COUT("\n[Module Details]");
            PLG_COUT_FMT("  Runtime:     {}", module->GetRuntime().string());
        }

        // Directories
        const auto& dirs = module->GetDirectories();
        if (!dirs.empty()) {
            PLG_COUT_FMT("\n[Search Directories] ({} total)", dirs.size());
            for (size_t i = 0; i < std::min(dirs.size(), size_t(5)); ++i) {
                PLG_COUT_FMT("  - {}", dirs[i].string());
            }
            if (dirs.size() > 5) {
                PLG_COUT_FMT("  ... and {} more directories", dirs.size() - 5);
            }
        }

        // Assembly Information
        if (auto assembly = module->GetAssembly()) {
            PLG_COUT("\n[Assembly Information]");
            PLG_COUT("  Assembly loaded and active");
        }

        // Platforms
        const auto& platforms = module->GetPlatforms();
        if (!platforms.empty()) {
            PLG_COUT("\n[Supported Platforms]");
            PLG_COUT("  " + plg::join(platforms, ", "));
        }

        // Dependencies
        const auto& deps = module->GetDependencies();
        if (!deps.empty()) {
            PLG_COUT_FMT("\n[Dependencies] ({} total)", deps.size());
            for (const auto& dep : deps) {
                PLG_COUT_FMT("  - {} ({})", dep.GetName(), dep.GetConstraints());
                if (dep.IsOptional()) {
                    PLG_COUT("    └─ Optional");
                }
            }
        }

        // Conflicts
        const auto& conflicts = module->GetConflicts();
        if (!conflicts.empty()) {
            PLG_COUT_FMT("\n[Conflicts] ({} total)", conflicts.size());
            for (const auto& conflict : conflicts) {
                PLG_COUT_FMT("  - {} ({})", conflict.GetName(), conflict.GetConstraints());
                const auto& reason = conflict.GetReason();
                if (!reason.empty()) {
                    PLG_COUT_FMT("    └─ Reason: {}", reason);
                }
            }
        }

        // Performance Information
        PLG_COUT("\n[Performance Metrics]");
        PLG_COUT_FMT("  Total Time:  {}", FormatDuration(module->GetTotalTime()));

        // Show timing for different operations
        ExtensionState operations[] = {
            ExtensionState::Parsing,
            ExtensionState::Resolving,
            ExtensionState::Loading,
            ExtensionState::Starting
        };

        for (const auto& op : operations) {
            try {
                if (auto duration = module->GetOperationTime(op); duration.count() > 0) {
                    PLG_COUT_FMT("  {:<12} {}", plg::enum_to_string(op) + ":"s, FormatDuration(duration));
                }
            } catch (...) {}
        }

        // Errors and Warnings
        if (module->HasErrors() || !module->GetWarnings().empty()) {
            PLG_COUT("\n[Issues]");
            for (const auto& error : module->GetErrors()) {
                PLG_CERR_FMT("  ERROR: {}", error);
            }
            for (const auto& warning : module->GetWarnings()) {
                PLG_COUT_FMT("  WARNING: {}", warning);
            }
        }

        PLG_COUT(std::string(80, '='));
    }
#endif

    void Update() {
        if (plug->IsInitialized()) {
            plug->Update();
        }
    }

    bool IsInitialized() const {
        return plug->IsInitialized();
    }

private:
    bool CheckManager() {
        if (!plug->IsInitialized()) {
            PLG_CERR("Initialize system before use.");
            return false;
        }
        const auto& manager = plug->GetManager();
        if (!manager.IsInitialized()) {
            PLG_CERR("You must load plugin manager before query any information from it.");
            return false;
        }
        return true;
    }

    std::shared_ptr<Plugify> plug;
};

void RunInteractiveMode(PlugifyApp& app) {
    PLG_COUT("Entering interactive mode. Type 'help' for commands or 'exit' to quit.");
    
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        
        // Parse the line using CLI11
        std::vector<std::string> args;
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            args.push_back(word);
        }
        
        if (args.empty()) continue;
        
        // Handle exit commands
        if (args[0] == "exit" || args[0] == "quit" || args[0] == "-e" || args[0] == "-q") {
            break;
        }
        
        // Create a temporary CLI app for parsing the interactive command
        CLI::App interactiveApp{"Interactive command"};
        interactiveApp.allow_extras();
        interactiveApp.prefix_command();
        
        // Define commands
        auto* init_cmd = interactiveApp.add_subcommand("init", "Initialize plugin system");
        auto* term_cmd = interactiveApp.add_subcommand("term", "Terminate plugin system");
        auto* version_cmd = interactiveApp.add_subcommand("version", "Show version information");
        auto* load_cmd = interactiveApp.add_subcommand("load", "Load plugin manager");
        auto* unload_cmd = interactiveApp.add_subcommand("unload", "Unload plugin manager");
        auto* reload_cmd = interactiveApp.add_subcommand("reload", "Reload plugin manager");
        auto* plugins_cmd = interactiveApp.add_subcommand("plugins", "List all plugins");
        auto* modules_cmd = interactiveApp.add_subcommand("modules", "List all modules");
        
        auto* plugin_cmd = interactiveApp.add_subcommand("plugin", "Show plugin information");
        std::string plugin_name;
        bool plugin_use_id = false;
        plugin_cmd->add_option("name", plugin_name, "Plugin name or ID")->required();
        plugin_cmd->add_flag("-u,--uuid", plugin_use_id, "Use ID instead of name");
        
        auto* module_cmd = interactiveApp.add_subcommand("module", "Show module information");
        std::string module_name;
        bool module_use_id = false;
        module_cmd->add_option("name", module_name, "Module name or ID")->required();
        module_cmd->add_flag("-u,--uuid", module_use_id, "Use ID instead of name");
        
        auto* help_cmd = interactiveApp.add_subcommand("help", "Show help");
        
        // Set callbacks
        init_cmd->callback([&app]() { app.Initialize(); });
        term_cmd->callback([&app]() { app.Terminate(); });
        version_cmd->callback([&app]() { app.ShowVersion(); });
        load_cmd->callback([&app]() { app.LoadManager(); });
        unload_cmd->callback([&app]() { app.UnloadManager(); });
        reload_cmd->callback([&app]() { app.ReloadManager(); });
        plugins_cmd->callback([&app]() { app.ListPlugins(); });
        modules_cmd->callback([&app]() { app.ListModules(); });
        plugin_cmd->callback([&app, &plugin_name, &plugin_use_id]() {
            app.ShowPlugin(plugin_name, plugin_use_id);
        });
        module_cmd->callback([&app, &module_name, &module_use_id]() { 
            app.ShowModule(module_name, module_use_id); 
        });
        
        help_cmd->callback([]() {
            PLG_COUT("Plugify Interactive Mode Commands:");
            PLG_COUT("  init           - Initialize plugin system");
            PLG_COUT("  term           - Terminate plugin system");
            PLG_COUT("  version        - Show version information");
            PLG_COUT("  load           - Load plugin manager");
            PLG_COUT("  unload         - Unload plugin manager");
            PLG_COUT("  reload         - Reload plugin manager");
            PLG_COUT("  plugins        - List all plugins");
            PLG_COUT("  modules        - List all modules");
            PLG_COUT("  plugin <name>  - Show plugin information (use -u for ID)");
            PLG_COUT("  module <name>  - Show module information (use -u for ID)");
            PLG_COUT("  help           - Show this help");
            PLG_COUT("  exit/quit      - Exit interactive mode");
        });
        
        try {
            interactiveApp.parse(args);
        } catch (const CLI::ParseError& e) {
            if (e.get_exit_code() != 0) {
                PLG_CERR_FMT("Error: {}", e.what());
                PLG_CERR("Type 'help' for available commands.");
            }
        }
        
        app.Update();
    }
}

int main(int argc, char** argv) {
    auto plugify = MakePlugify();
    if (!plugify) {
        PLG_COUT(plugify.error());
        return EXIT_FAILURE;
    }
    
    PlugifyApp app(std::move(*plugify));
    
    CLI::App cliApp{"Plugify - Plugin Management System"};
    cliApp.set_version_flag("-v,--version", [&app]() {
        app.ShowVersion();
        return "";
    });
    
    // Global options
    bool interactive = false;
    cliApp.add_flag("-i,--interactive", interactive, "Run in interactive mode");
    
    // Define subcommands
    auto* init_cmd = cliApp.add_subcommand("init", "Initialize plugin system");
    auto* term_cmd = cliApp.add_subcommand("term", "Terminate plugin system");
    auto* load_cmd = cliApp.add_subcommand("load", "Load plugin manager");
    auto* unload_cmd = cliApp.add_subcommand("unload", "Unload plugin manager");
    auto* reload_cmd = cliApp.add_subcommand("reload", "Reload plugin manager");
    auto* plugins_cmd = cliApp.add_subcommand("plugins", "List all plugins");
    auto* modules_cmd = cliApp.add_subcommand("modules", "List all modules");
    
    auto* plugin_cmd = cliApp.add_subcommand("plugin", "Show plugin information");
    std::string plugin_name;
    bool plugin_use_id = false;
    plugin_cmd->add_option("name", plugin_name, "Plugin name or ID")->required();
    plugin_cmd->add_flag("-u,--uuid", plugin_use_id, "Use ID instead of name");
    
    auto* module_cmd = cliApp.add_subcommand("module", "Show module information");
    std::string module_name;
    bool module_use_id = false;
    module_cmd->add_option("name", module_name, "Module name or ID")->required();
    module_cmd->add_flag("-u,--uuid", module_use_id, "Use ID instead of name");
    
    // Set callbacks for commands
    init_cmd->callback([&app]() {
        app.Initialize();
        app.Update();
    });
    
    term_cmd->callback([&app]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }
        app.Terminate();
    });
    
    load_cmd->callback([&app]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }
        app.LoadManager();
        app.Update();
    });
    
    unload_cmd->callback([&app]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }
        app.UnloadManager();
        app.Update();
    });
    
    reload_cmd->callback([&app]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }
        app.ReloadManager();
        app.Update();
    });
    
    plugins_cmd->callback([&app]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }
        app.ListPlugins();
    });
    
    modules_cmd->callback([&app]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }
        app.ListModules();
    });
    
    plugin_cmd->callback([&app, &plugin_name, &plugin_use_id]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }
        app.ShowPlugin(plugin_name, plugin_use_id);
    });
    
    module_cmd->callback([&app, &module_name, &module_use_id]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }
        app.ShowModule(module_name, module_use_id);
    });
    
    // Parse command line arguments
    try {
        cliApp.parse(argc, argv);
        
        // If no subcommand was provided or interactive flag is set, run interactive mode
        if (interactive || (argc == 1 || !cliApp.get_subcommands().empty())) {
            RunInteractiveMode(app);
        }
    } catch (const CLI::ParseError& e) {
        return cliApp.exit(e);
    } catch (const std::exception& e) {
        PLG_CERR_FMT("Error: {}", e.what());
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

// ============================================================================
// Usage Examples
// ============================================================================
/*
// Example 1: Simple usage with defaults
int main() {
    auto result = plugify::MakePlugify("./app");
    if (!result) {
        std::cerr << "Failed to create Plugify: " << result.error().message << std::endl;
        return 1;
    }

    auto plugify = result.value();
    if (auto initResult = plugify->Initialize(); !initResult) {
        std::cerr << "Failed to initialize: " << initResult.error().message << std::endl;
        return 1;
    }

    // Load extensions
    const auto& manager = plugify->GetManager();
    manager->Initialize();

    // Main loop
    bool running = true;
    while (running) {
        plugify->Update();
        // Your application logic here
    }

    plugify->Terminate();
    return 0;
}

// Example 2: Advanced configuration
int main() {
    // Create custom logger
    auto logger = std::make_shared<plugify::impl::AsyncLogger>(
        std::make_shared<plugify::impl::FileLogger>("./logs/app.log")
    );

    // Configure plugin manager
    plugify::PluginManagerConfig config;
    config.paths.baseDir = "./app";
    config.paths.extensionsDir = "extensions";
    config.loading.enableHotReload = true;
    config.loading.parallelLoading = true;
    config.runtime.updateInterval = std::chrono::milliseconds(16);

    // Build Plugify instance
    auto result = plugify::Plugify::CreateBuilder()
        .WithConfig(config)
        .WithLogger(logger)
        .WithService<MyCustomService>(std::make_shared<MyCustomServiceImpl>())
        .Build();

    if (!result) {
        std::cerr << "Failed: " << result.error().message << std::endl;
        return 1;
    }

    auto plugify = result.value();

    // Initialize asynchronously
    auto initFuture = plugify->InitializeAsync();

    // Do other initialization...

    // Wait for initialization
    if (auto initResult = initFuture.get(); !initResult) {
        std::cerr << "Failed to initialize: " << initResult.error().message << std::endl;
        return 1;
    }

    // Access provider for convenient operations
    auto provider = plugify->GetProvider();
    provider->LogInfo("Application started");

    // Main loop
    bool running = true;
    while (running) {
        plugify->Update();
    }

    plugify->Terminate();
    return 0;
}
*/