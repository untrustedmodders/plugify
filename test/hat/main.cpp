#include "std_logger.h"
#include <wizard/wizard.h>
#include <wizard/module.h>
#include <wizard/package.h>
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

void printHelp() {
	std::cout <<
		"\nWizard"
		"\n(c) untrustedmodders"
		"\nhttps://github.com/untrustedmodders"
		"\nusage: wzd -<command> [arguments]"
		"\n"
		"\nGeneral commands:"
		"\n  -init           Initialize wizard"
		"\n  -term           Terminate wizard"
		"\n  -help           Print help"
		"\n  -version        Version information"
		"\n  -exit           Exit"
		"\n"
		"\nPlugin Manager commands:"
		//"\n  -plugin <plugin-name> - Show information about a module"
		//"\n  -module <module-name> - Show information about a plugin"
		"\n  -modules        List running modules"
		"\n  -plugins        List running plugins"
		"\n"
		"\nPackage Manager commands:"
		"\n  -S <package-name>     Packages to install (space separated)"
		"\n  -Sf </path/to/file>   Packages to install (from file manifest)"
		"\n  -Sw <link/to/file>    Packages to install (from HTTP manifest)"
		"\n  -R <package-name>     Packages to uninstall (space separated)"
		"\n  -Ra                   Uninstall all packages"
		"\n  -U <package-name>     Packages to update (space separated)"
		"\n  -Ua                   Update all packages"
		"\n  -Q                    Print all local packages"
		"\n  -Qr                   Print all remote packages"
		"\n  -Qi <package-name>    Show information about local package"
		"\n  -Qri <package-name>   Show information about remote package"
		"\n  -Qd                   Snapshot packages into manifest"
	<< std::endl;
}


int main(int argc, const char** argv) {
    std::span<const char*> arg{argv, static_cast<size_t>(argc)};
    std::shared_ptr<wizard::IWizard> sorcerer = wizard::MakeWizard();
    if (sorcerer) {
        auto logger = std::make_shared<sorcerer::StdLogger>();
        logger->SetSeverity(wizard::Severity::Debug);
        sorcerer->SetLogger(std::move(logger));
        bool running = true;
        while (running) {
            std::string command;
            std::getline(std::cin, command);
            std::vector<std::string> args = Split(command, ' ');
            if (args.empty())
                continue;

            if (args[0] == "e" || args[0] == "exit" || args[0] == "q" || args[0] == "quit") {
                running = false;
            } else if (args[0] == "wzd" && args.size() > 1) {
                if (args[1] == "-init") {
                    if (!sorcerer->Initialize(arg)) {
                        sorcerer->Log("No feet, no sweets!", wizard::Severity::Error);
                        return EXIT_FAILURE;
                    }
                } else if (args[1] == "-term") {
                    sorcerer->Terminate();
                } else if (args[1] == "-exit") {
					running = false;
                } else if (args[1] == "-help") {
					printHelp();
                }  else if (args[1] == "-version") {
					std::cout << "Wizard " << sorcerer->GetVersion().ToString();
                } else if (args[1] == "-modules") {
                    const char separator = ' ';
                    const int nameWidth = 20;
                    const int numWidth = 10;
                    const int fullNameWidth = 40;
                    const int statusWidth = 25;

                    PrintElement("Lang", numWidth, separator);
                    PrintElement("Name", nameWidth, separator);
                    PrintElement("FriendlyName", fullNameWidth, separator);
                    PrintElement("Status", statusWidth, separator);

                    std::cout << std::endl;

                    if (auto pluginManager = sorcerer->GetPluginManager().lock()) {
                        for (auto& module : pluginManager->GetModules()) {
                            PrintElement(module->GetDescriptor().language, numWidth, separator);
                            PrintElement(module->GetName(), nameWidth, separator);
                            PrintElement(module->GetFriendlyName(), fullNameWidth, separator);
                            PrintElement(wizard::ModuleStateToString(module->GetState()), statusWidth, separator);
                            std::cout << std::endl;
                        }
                    }
                } else if (args[1] == "-plugins") {
                    const char separator = ' ';
					const int nameWidth = 20;
					const int numWidth = 10;
					const int fullNameWidth = 40;
					const int statusWidth = 25;

					PrintElement("Lang", numWidth, separator);
					PrintElement("Name", nameWidth, separator);
					PrintElement("FriendlyName", fullNameWidth, separator);
					PrintElement("Status", statusWidth, separator);

                    std::cout << std::endl;

                    if (auto pluginManager = sorcerer->GetPluginManager().lock()) {
                        for (auto& plugin : pluginManager->GetPlugins()) {
                            PrintElement(plugin->GetId(), numWidth, separator);
                            PrintElement(plugin->GetName(), nameWidth, separator);
                            PrintElement(plugin->GetFriendlyName(), fullNameWidth, separator);
                            PrintElement(wizard::PluginStateToString(plugin->GetState()), statusWidth, separator);
                            std::cout << std::endl;
                        }
                    }
                } else if (args[1] == "-Qd") {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->SnapshotPackages(sorcerer->GetConfig().baseDir / std::format("snapshot_{}.wpackagemanifest", wizard::DateTime::Get("%Y_%m_%d_%H_%M_%S")), true);
					}
				} else if (args[1] == "-S" && args.size() > 2) {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->InstallPackages(std::span(args.begin() + 2, args.size() - 2));
					}
				} else if (args[1] == "-Sf" && args.size() > 2) {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->InstallAllPackages(fs::path{args[2]}, args.size() > 3);
					}
				} else if (args[1] == "-Sw" && args.size() > 2) {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->InstallAllPackages(args[2], args.size() > 3);
					}
				} else if (args[1] == "-R" && args.size() > 2) {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->UninstallPackages(std::span(args.begin() + 2, args.size() - 2));
					}
				} else if (args[1] == "-Ra") {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->UninstallAllPackages();
					}
				} else if (args[1] == "-U" && args.size() > 2) {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->UpdatePackages(std::span(args.begin() + 2, args.size() - 2));
					}
				} else if (args[1] == "-Ua") {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->UpdateAllPackages();
					}
				} else if (args[1] == "-Q") {
					const char separator = ' ';
					const int nameWidth = 20;
					const int numWidth = 6;
					const int pathWidth = 60;

					PrintElement("Name", nameWidth, separator);
					PrintElement("Type", nameWidth, separator);
					PrintElement("Path", pathWidth, separator);
					PrintElement("Version", numWidth, separator);

					std::cout << std::endl;

					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						for (const auto& packageRef : packageManager->GetLocalPackages()) {
							const auto& package = packageRef.get();
							PrintElement(package.name, nameWidth, separator);
							PrintElement(package.type, nameWidth, separator);
							PrintElement(package.path, pathWidth, separator);
							PrintElement(package.version, numWidth, separator);
							std::cout << std::endl;
						}
					}
				} else if (args[1] == "-Qr") {
					const char separator = ' ';
					const int nameWidth = 20;
					const int descWidth = 80;

					PrintElement("Name", nameWidth, separator);
					PrintElement("Type", nameWidth, separator);
					PrintElement("Author", nameWidth, separator);
					PrintElement("Description", descWidth, separator);

					std::cout << std::endl;

					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						for (const auto& packageRef : packageManager->GetRemotePackages()) {
							const auto& package = packageRef.get();
							PrintElement(package.name, nameWidth, separator);
							PrintElement(package.type, nameWidth, separator);
							PrintElement(package.author, nameWidth, separator);
							PrintElement(package.description, descWidth, separator);
							std::cout << std::endl;
						}
					}

				} else if (args[1] == "-Qi" && args.size() > 2) {
					const char separator = ' ';
					const int nameWidth = 20;
					const int numWidth = 6;
					const int pathWidth = 60;

					PrintElement("Name", nameWidth, separator);
					PrintElement("Type", nameWidth, separator);
					PrintElement("Path", pathWidth, separator);
					PrintElement("Version", numWidth, separator);

					std::cout << std::endl;

					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						for (const auto& packageRef : packageManager->GetLocalPackages()) {
							const auto& package = packageRef.get();
							if (package.name == args[2]) {
								PrintElement(package.name, nameWidth, separator);
								PrintElement(package.type, nameWidth, separator);
								PrintElement(package.path, pathWidth, separator);
								PrintElement(package.version, numWidth, separator);
								std::cout << std::endl;
								std::cout << "Descriptor: ";
								std::cout << "\n\t File Version: " << package.descriptor->fileVersion;
								std::cout << "\n\t Version: " << package.descriptor->version;
								std::cout << "\n\t Version Name: " << package.descriptor->versionName;
								std::cout << "\n\t Friendly Name: " << package.descriptor->friendlyName;
								std::cout << "\n\t Description: " << package.descriptor->description;
								std::cout << "\n\t Created By: " << package.descriptor->createdBy;
								std::cout << "\n\t Created By URL: " << package.descriptor->createdByURL;
								std::cout << "\n\t Docs URL: " << package.descriptor->docsURL;
								std::cout << "\n\t Download URL: " << package.descriptor->downloadURL;
								std::cout << "\n\t Update URL: " << package.descriptor->updateURL;
								std::cout << "\n\t Supported platforms: ";
								if (package.descriptor->supportedPlatforms.empty())
									std::cout << "[any]";
								else
								{
									for (const auto& platform : package.descriptor->supportedPlatforms) {
										std::cout << "[" << platform << "] ";
									}
								}
								std::cout << std::endl;
								break;
							}
						}
					}
				} else if (args[1] == "-Qri" && args.size() > 2) {
					const char separator = ' ';
					const int nameWidth = 20;
					const int descWidth = 80;

					PrintElement("Name", nameWidth, separator);
					PrintElement("Type", nameWidth, separator);
					PrintElement("Author", nameWidth, separator);
					PrintElement("Description", descWidth, separator);

					std::cout << std::endl;

					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						for (const auto& packageRef : packageManager->GetRemotePackages()) {
							const auto& package = packageRef.get();
							if (package.name == args[2]) {
								PrintElement(package.name, nameWidth, separator);
								PrintElement(package.type, nameWidth, separator);
								PrintElement(package.author, nameWidth, separator);
								PrintElement(package.description, descWidth, separator);
								std::cout << std::endl;
								std::cout << "Versions: ";
								for (const auto& version : package.versions) {
									std::cout << "[" << version.version << "] ";
								}
								std::cout << std::endl;
								break;
							}
						}
					}
				}
			}
        }
    }

    return EXIT_SUCCESS;
}