#include "std_logger.h"
#include <wizard/wizard.h>
#include <wizard/module.h>
#include <wizard/package_manager.h>
#include <wizard/plugin_manager.h>
#include <wizard/plugin.h>
#include <wizard/plugin_descriptor.h>

#include <iostream>

std::vector<std::string> Split(const std::string& str, char sep) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream is{str};

    while (std::getline(is, token, sep))
        tokens.emplace_back(token.c_str());
    return tokens;
}

template<typename T>
void PrintElement(T t, int width, char separator) {
    std::cout << std::left << std::setw(width) << std::setfill(separator) << t;
}

template<typename T>
void PrintArrayElement(const std::vector<T>& t, int width, char separator) {
    std::cout << std::left << std::setw(width) << std::setfill(separator);
    for (const auto& e : t) {
        std::cout << e << " ";
    }
}

int main(int argc, const char** argv) {
    std::span<const char*> arg{argv, static_cast<size_t>(argc)};
    std::shared_ptr<wizard::IWizard> sorcerer = wizard::MakeWizard();
    if (sorcerer) {
        auto logger = std::make_shared<sorcerer::StdLogger>();
        logger->SetSeverity(wizard::Severity::Debug);
        sorcerer->SetLogger(std::move(logger));

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
            } else if (args[0] == "wzd" && args.size() > 1) {
                if (args[1] == "init") {
                    if (!sorcerer->Initialize(arg)) {
                        sorcerer->Log("No feet, no sweets!", wizard::Severity::Error);
                        return EXIT_FAILURE;
                    }
                } else if (args[1] == "term") {
                    sorcerer->Terminate();
                } else if (args[1] == "help") {
                    std::cout << "Wizard Menu" << std::endl;
                    std::cout << "usage: wzd <command> [arguments]" << std::endl;
                    std::cout << "  plugin       - Information about a module" << std::endl;
                    std::cout << "  module       - Information about a plugin" << std::endl;
                    std::cout << "  modules      - List modules" << std::endl;
                    std::cout << "  plugins      - List plugins" << std::endl;
                    std::cout << "  snapshot     - Snapshot packages into manifest" << std::endl;
                    std::cout << "  update       - Update all packages" << std::endl;
                    std::cout << "  version      - Version information" << std::endl;
                } else if (args[1] == "modules") {
                    const char separator = ' ';
                    const int nameWidth = 20;
                    const int numWidth = 10;
                    const int statusWidth = 10;

                    PrintElement("Lang", numWidth, separator);
                    PrintElement("Name", nameWidth, separator);
                    PrintElement("FriendlyName", nameWidth + numWidth, separator);
                    PrintElement("Status", statusWidth, separator);

                    std::cout << std::endl;

                    if (auto pluginManager = sorcerer->GetPluginManager().lock()) {
                        for (auto& module : pluginManager->GetModules()) {
                            PrintElement(module->GetDescriptor().language, numWidth, separator);
                            PrintElement(module->GetName(), nameWidth, separator);
                            PrintElement(module->GetFriendlyName(), nameWidth + numWidth, separator);
                            PrintElement(wizard::ModuleStateToString(module->GetState()), statusWidth, separator);
                            std::cout << std::endl;
                        }
                    }
                } else if (args[1] == "plugins") {
                    const char separator = ' ';
                    const int nameWidth = 20;
                    const int numWidth = 6;
                    const int statusWidth = 10;

                    PrintElement("Id", numWidth, separator);
                    PrintElement("Name", nameWidth, separator);
                    PrintElement("FriendlyName", nameWidth, separator);
                    PrintElement("Status", statusWidth, separator);

                    std::cout << std::endl;

                    if (auto pluginManager = sorcerer->GetPluginManager().lock()) {
                        for (auto& plugin : pluginManager->GetPlugins()) {
                            PrintElement(plugin->GetId(), numWidth, separator);
                            PrintElement(plugin->GetName(), nameWidth, separator);
                            PrintElement(plugin->GetFriendlyName(), nameWidth, separator);
                            PrintElement(wizard::PluginStateToString(plugin->GetState()), statusWidth, separator);
                            std::cout << std::endl;
                        }
                    }
                } else if (args[1] == "snapshot") {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->SnapshotPackages(sorcerer->GetConfig().baseDir / std::format("snapshot_{}.wpackagemanifest", wizard::DateTime::Get("%Y_%m_%d_%H_%M_%S")), true);
					}
				} else if (args[1] == "update") {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->UpdateAllPackages();
					}
				} else if (args[1] == "install" && args.size() > 2) {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->InstallAllPackages(args[2], args.size() > 3);
					}
				}
            }
        }
    }

    return EXIT_SUCCESS;
}
