#include "plugify/api/file_system.hpp"
#include "plugify/asm/assembly_loader.hpp"

#include <../../include/plugify/core/plugify.hpp>
#include <CLI/CLI.hpp>
#include <plg/format.hpp>
#include <plugify/api/date_time.hpp>
#include <plugify/api/dependency_handle.hpp>
#include <plugify/api/manager.hpp>
#include <plugify/api/module_handle.hpp>
#include <plugify/api/module_manifest_handle.hpp>
#include <plugify/api/plugin_handle.hpp>
#include <plugify/api/plugin_manifest_handle.hpp>

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <plugify/api/log.hpp>
#include <sstream>
#include <unordered_set>

#define PLG_LOG(x) std::cout << x << std::endl
#define PLG_ERROR(x) std::cerr << x << std::endl
#define PLG_LOG_FMT(...) std::cout << std::format(__VA_ARGS__) << std::endl
#define PLG_ERROR_FMT(...) std::cerr << std::format(__VA_ARGS__) << std::endl

int32_t FormatInt(const std::string& str) {
	int32_t result;
	auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);

	if (ec != std::errc{}) {
		PLG_ERROR_FMT("Error: {}", std::make_error_code(ec).message());
		return -1;
	} else if (ptr != str.data() + str.size()) {
		PLG_ERROR("Invalid argument: trailing characters after the valid part");
		return -1;
	}

	return result;
}

template<typename S, typename T, typename F>
void Print(T t, F&& f, std::string_view tab = "  ") {
	std::string result(tab);
	if (t.GetState() != S::Loaded) {
		std::format_to(std::back_inserter(result), "[{:02d}] <{}> {}", t.GetId(), f(t.GetState()), t.GetName());
	} else {
		std::format_to(std::back_inserter(result), "[{:02d}] {}", t.GetId(), t.GetName());
	}
	auto manifest = t.GetManifest();
	std::format_to(std::back_inserter(result), " ({})", manifest.GetVersion());
	const auto& author = manifest.GetAuthor();
	if (!author.empty()) {
		std::format_to(std::back_inserter(result), " by {}", author);
	}
	PLG_LOG(result);
}

template<typename S, typename T, typename F>
void Print(std::string_view name, T t, F&& f) {
	if (t.GetState() == S::Error) {
		PLG_ERROR_FMT("{} has error: {}.", name, t.GetError());
	} else {
		PLG_LOG_FMT("{} {} is {}.", name, t.GetId(), f(t.GetState()));
	}
	auto manifest = t.GetManifest();
	const auto& author = manifest.GetAuthor();
	if (!author.empty()) {
		PLG_LOG_FMT("  Name: \"{}\" by {}", t.GetName(), author);
	} else {
		PLG_LOG_FMT("  Name: \"{}\"", t.GetName());
	}
	PLG_LOG_FMT("  Version: {}", manifest.GetVersion());
	const auto& description = manifest.GetDescription();
	if (!description.empty()) {
		PLG_LOG_FMT("  Description: {}", description);
	}
	const auto& website = manifest.GetWebsite();
	if (!website.empty()) {
		PLG_LOG_FMT("  Website: {}", website);
	}
	const auto& license = manifest.GetLicense();
	if (!license.empty()) {
		PLG_LOG_FMT("  License: {}", license);
	}
}

using namespace plugify;

class PlugifyApp {
public:
    PlugifyApp() : plug(MakePlugify()) {
        if (plug) {
            logger = std::make_shared<Logger>();
            plug.SetLogger(logger);
            logger->SetSeverity(Severity::Debug);
            plug.SetAssemblyLoader(std::make_shared<AssemblyLoader>());
            plug.SetFileSystem(std::make_shared<StandardFileSystem>());
        }
    }

    void Initialize() {
        if (!plug.Initialize()) {
            PLG_ERROR("No feet, no sweets!");
            throw std::runtime_error("Failed to initialize Plugify");
        }
        logger->SetSeverity(plug.GetConfig().logSeverity.value_or(Severity::Debug));
        plug.GetManager().Initialize();
        PLG_LOG("Plugin system initialized.");
    }

    void Terminate() {
        plug.Terminate();
        PLG_LOG("Plugin system terminated.");
    }

    void ShowVersion() {
        static std::string copyright = std::format("Copyright (C) 2023-{}{}{}{} Untrusted Modders Team", 
                                                   __DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10]);
        PLG_LOG(R"(      ____)" "");
        PLG_LOG(R"( ____|    \         Plugify )" << plug.GetVersion().to_string());
        PLG_LOG(R"((____|     `._____  )" << copyright);
        PLG_LOG(R"( ____|       _|___)" "");
        PLG_LOG(R"((____|     .'       This program may be freely redistributed under)" "");
        PLG_LOG(R"(     |____/         the terms of the MIT License.)" "");
    }

    void LoadManager() {
        if (!plug.IsInitialized()) {
            PLG_ERROR("Initialize system before use.");
            return;
        }
        auto manager = plug.GetManager();
        if (manager.IsInitialized()) {
            PLG_ERROR("Plugin manager already loaded.");
        } else {
            manager.Initialize();
            PLG_LOG("Plugin manager was loaded.");
        }
    }

    void UnloadManager() {
        if (!plug.IsInitialized()) {
            PLG_ERROR("Initialize system before use.");
            return;
        }
        auto manager = plug.GetManager();
        if (!manager.IsInitialized()) {
            PLG_ERROR("Plugin manager already unloaded.");
        } else {
            manager.Terminate();
            PLG_LOG("Plugin manager was unloaded.");
        }
    }

    void ReloadManager() {
        if (!plug.IsInitialized()) {
            PLG_ERROR("Initialize system before use.");
            return;
        }
        auto manager = plug.GetManager();
        if (!manager.IsInitialized()) {
            PLG_ERROR("Plugin manager not loaded.");
        } else {
            manager.Terminate();
            manager.Initialize();
            PLG_LOG("Plugin manager was reloaded.");
        }
    }

    void ListPlugins() {
        if (!CheckManager()) return;
        
        auto manager = plug.GetManager();
        auto plugins = manager.GetPlugins();
        auto count = plugins.size();
        if (!count) {
            PLG_ERROR("No plugins loaded.");
        } else {
            PLG_LOG_FMT("Listing {} plugin{}:", count, (count > 1) ? "s" : "");
            for (const auto& plugin : plugins) {
                Print<PluginState>(plugin, PluginUtils::ToString);
            }
        }
    }

    void ListModules() {
        if (!CheckManager()) return;
        
        auto manager = plug.GetManager();
        auto modules = manager.GetModules();
        auto count = modules.size();
        if (!count) {
            PLG_ERROR("No modules loaded.");
        } else {
            PLG_LOG_FMT("Listing {} module{}:", count, (count > 1) ? "s" : "");
            for (const auto& module : modules) {
                Print<ModuleState>(module, ModuleUtils::ToString);
            }
        }
    }

    void ShowPlugin(const std::string& name, bool useId) {
        if (!CheckManager()) return;
        
        auto manager = plug.GetManager();
        auto plugin = useId ? manager.FindPluginFromId(FormatInt(name)) : manager.FindPlugin(name);
        if (plugin) {
            Print<PluginState>("Plugin", plugin, PluginUtils::ToString);
            auto manifest = plugin.GetManifest();
            PLG_LOG_FMT("  Language: {}", manifest.GetLanguage());
            PLG_LOG("  Dependencies: ");
            for (const auto& reference : manifest.GetDependencies()) {
                if (auto dependency = manager.FindPlugin(reference.GetName())) {
                    Print<PluginState>(dependency, PluginUtils::ToString, "    ");
                } else {
                    auto constraints = reference.GetConstrants();
                    for (const auto& [type, version] : constraints) {
                        PLG_LOG_FMT("    {} <Missing> ({}-v{})", reference.GetName(), 
                                   static_cast<int>(type), version.to_string());
                    }
                }
            }
            PLG_LOG_FMT("  File: {}", manifest.GetEntry());
        } else {
            PLG_ERROR_FMT("Plugin {} not found.", name);
        }
    }

    void ShowModule(const std::string& name, bool useId) {
        if (!CheckManager()) return;
        
        auto manager = plug.GetManager();
        auto module = useId ? manager.FindModuleFromId(FormatInt(name)) : manager.FindModule(name);
        if (module) {
            Print<ModuleState>("Module", module, ModuleUtils::ToString);
            PLG_LOG_FMT("  Language: {}", module.GetLanguage());
        } else {
            PLG_ERROR_FMT("Module {} not found.", name);
        }
    }

    void Update() {
        if (plug.IsInitialized()) {
            plug.Update();
        }
    }

    bool IsInitialized() const {
        return plug.IsInitialized();
    }

private:
    bool CheckManager() {
        if (!plug.IsInitialized()) {
            PLG_ERROR("Initialize system before use.");
            return false;
        }
        auto manager = plug.GetManager();
        if (!manager.IsInitialized()) {
            PLG_ERROR("You must load plugin manager before query any information from it.");
            return false;
        }
        return true;
    }

    PlugifyHandle plug;
    std::shared_ptr<Logger> logger;
};

void RunInteractiveMode(PlugifyApp& app) {
    PLG_LOG("Entering interactive mode. Type 'help' for commands or 'exit' to quit.");
    
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
            PLG_LOG("Plugify Interactive Mode Commands:");
            PLG_LOG("  init           - Initialize plugin system");
            PLG_LOG("  term           - Terminate plugin system");
            PLG_LOG("  version        - Show version information");
            PLG_LOG("  load           - Load plugin manager");
            PLG_LOG("  unload         - Unload plugin manager");
            PLG_LOG("  reload         - Reload plugin manager");
            PLG_LOG("  plugins        - List all plugins");
            PLG_LOG("  modules        - List all modules");
            PLG_LOG("  plugin <name>  - Show plugin information (use -u for ID)");
            PLG_LOG("  module <name>  - Show module information (use -u for ID)");
            PLG_LOG("  help           - Show this help");
            PLG_LOG("  exit/quit      - Exit interactive mode");
        });
        
        try {
            interactiveApp.parse(args);
        } catch (const CLI::ParseError& e) {
            if (e.get_exit_code() != 0) {
                PLG_ERROR_FMT("Error: {}", e.what());
                PLG_ERROR("Type 'help' for available commands.");
            }
        }
        
        app.Update();
    }
}

int main(int argc, char** argv) {
    PlugifyApp app;
    
    CLI::App cliApp{"Plugify - Plugin Management System"};
    cliApp.set_version_flag("-v,--version", []() {
        PlugifyApp tempApp;
        tempApp.ShowVersion();
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
        PLG_ERROR_FMT("Error: {}", e.what());
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}