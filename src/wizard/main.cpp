#include "plugin_manager.h"

template<typename T> void PrintElement(T t, int width, char separator) {
    std::cout << std::left << std::setw(width) << std::setfill(separator) << t;
}

std::vector<std::string> Split(const std::string& str, char sep) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream is{str};

    while (std::getline(is, token, sep))
        tokens.emplace_back(token.c_str());
    return tokens;
}

int main() {
    wizard::PluginManager& pluginManager = wizard::PluginManager::Get();
    pluginManager.loadAll();

    std::cout << "wzd <command> [arguments]" << std::endl;

    bool running = true;
    while (running) {
        std::string command;
        std::getline(std::cin, command);
        std::vector<std::string> args = Split(command, ' ');
        if (args.empty())
            continue;

        if (args[0] == "exit") {
            running = false;
        } else if (args[0] == "help") {
            std::cout << "Wizard Menu" << std::endl;
            std::cout << "usage: wzd <command> [arguments]" << std::endl;
            //std::cout << "  alias        - List or set an alias" << std::endl;
            //std::cout << "  clear        - Unload all plugins forcefully" << std::endl;
            //std::cout << "  cmds         - Show plugin commands" << std::endl;
            //std::cout << "  cvars        - Show plugin cvars" << std::endl;
            //std::cout << "  credits      - About Metamod:Source" << std::endl;
            //std::cout << "  force_unload - Forcefully unload a plugin" << std::endl;
            //std::cout << "  game         - Information about GameDLL" << std::endl;
            std::cout << "  info         - Information about a plugin" << std::endl;
            std::cout << "  list         - List plugins" << std::endl;
            //std::cout << "  load         - Load a plugin" << std::endl;
            //std::cout << "  pause        - Pause a running plugin" << std::endl;
            //std::cout << "  refresh      - Reparse plugin files" << std::endl;
            //std::cout << "  retry        - Attempt to reload a plugin" << std::endl;
            //std::cout << "  unload       - Unload a loaded plugin" << std::endl;
            //std::cout << "  unpause      - Unpause a paused plugin" << std::endl;
            //std::cout << "  version      - Version information" << std::endl;
        } else if (args[0] == "wzd" && args.size() > 1) {
            if (args[1] == "list") {
                const char separator = ' ';
                const int nameWidth = 20;
                const int numWidth = 6;
                const int statusWidth = 10;

                PrintElement("Id", numWidth, separator);
                PrintElement("Name", nameWidth, separator);
                PrintElement("Version", nameWidth, separator);
                PrintElement("Author", nameWidth, separator);
                PrintElement("Status", statusWidth, separator);
                std::cout << std::endl;

                for (const auto& plugin : pluginManager) {
                    PrintElement(plugin.getId(), numWidth, separator);
                    PrintElement(plugin.getName(), nameWidth, separator);
                    PrintElement(plugin.getVersion(), nameWidth, separator);
                    PrintElement(plugin.getAuthor(), nameWidth, separator);
                    PrintElement(wizard::utils::GetStatusText(plugin.getStatus()), statusWidth, separator);
                    std::cout << std::endl;
                }
            } else if (args.size() > 2) {
                if (args[1] == "info") {
                    if (auto plugin = pluginManager.findPlugin(std::stoul(args[2]))) {
                        std::cout << "\t" << "Name: " << plugin->getName() << std::endl;
                        std::cout << "\t" << "Description: " << plugin->getDescription() << std::endl;
                        std::cout << "\t" << "Url: " << plugin->getUrl() << std::endl;
                        std::cout << "\t" << "Author: " << plugin->getAuthor() << std::endl;
                        std::cout << "\t" << "Version: " << plugin->getVersion() << std::endl;
                        std::cout << "\t" << "Path: " << plugin->getPath() << std::endl;
                    }
                }
            }
        }
    }

    return 0;
}
