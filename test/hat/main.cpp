#include "std_logger.h"
#include <wizard/wizard.h>
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

int main() {
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
            } else if (args[0] == "help") {
                std::cout << "Wizard Menu" << std::endl;
                std::cout << "usage: wzd <command> [arguments]" << std::endl;
                std::cout << "  plugin       - Information about a module" << std::endl;
                std::cout << "  module       - Information about a plugin" << std::endl;
                //std::cout << "  listm        - List modules" << std::endl;
                //std::cout << "  listp        - List plugins" << std::endl;
                std::cout << "  version      - Version information" << std::endl;
            } else if (args[0] == "wzd" && args.size() > 1) {
                if (args[1] == "init") {
                    if (!sorcerer->Initialize()) {
                        sorcerer->Log("No feet, no sweets!", wizard::Severity::Error);
                        return EXIT_FAILURE;
                    }
                } else if (args[1] == "term") {
                    sorcerer->Terminate();
                } else if (args[1] == "version") {
                    std::cout << "Version: " << sorcerer->GetVersion().ToString() << std::endl;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
