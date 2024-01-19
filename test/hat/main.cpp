#include "std_logger.h"
#include <wizard/wizard.h>
#include <wizard/module.h>
#include <wizard/package.h>
#include <wizard/package_manager.h>
#include <wizard/plugin_manager.h>
#include <wizard/plugin.h>
#include <wizard/plugin_descriptor.h>

#include <chrono>
#include <iostream>
#include <unordered_set>

std::vector<std::string> Split(const std::string& str, char sep) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream is{str};

    while (std::getline(is, token, sep))
        tokens.emplace_back(token.c_str());
    return tokens;
}

std::string FormatTime(std::string_view format = "%Y-%m-%d %H:%M:%S") {
	auto now = std::chrono::system_clock::now();
	auto timeT = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&timeT), format.data());
	return ss.str();
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
		"\nusage: wizard -<command> [arguments]"
		"\n"
		"\nGeneral commands:"
		"\n  -init           Initialize wizard"
		"\n  -term           Terminate wizard"
		"\n  -help           Print help"
		"\n  -version        Version information"
		"\n  -exit           Exit"
		"\n"
		"\nPlugin Manager commands:"
		"\n  -load                 - Load plugin manager"
		"\n  -unload               - Unload plugin manager"
		"\n  -plugin <plugin-name> - Show information about a module"
		"\n  -module <module-name> - Show information about a plugin"
		"\n  -modules              - List running modules"
		"\n  -plugins              - List running plugins"
		"\n"
		"\nPackage Manager commands:"
		"\n  -install <name>        - Packages to install (space separated)"
		"\n  -install --file <file> - Packages to install (from file manifest)"
		"\n  -install --link <link> - Packages to install (from HTTP manifest)"
		"\n  -uninstall <name>      - Packages to uninstall (space separated)"
		"\n  -uninstall --all       - Uninstall all packages"
		"\n  -update <name>         - Packages to update (space separated)"
		"\n  -update --all          - Update all packages"
		"\n  -list                  - Print all local packages"
		"\n  -query                 - Print all remote packages"
		"\n  -show <name>           - Show information about local package"
		"\n  -search <name>         - Show information about remote package"
		"\n  -snapshot              - Snapshot packages into manifest"
	<< std::endl;
}

int main() {
    std::shared_ptr<wizard::IWizard> sorcerer = wizard::MakeWizard();
    if (sorcerer) {
        auto logger = std::make_shared<sorcerer::StdLogger>();
        sorcerer->SetLogger(logger);
		logger->SetSeverity(wizard::Severity::Debug);
        bool running = true;
        while (running) {
            std::string command;
            std::getline(std::cin, command);
            std::vector<std::string> args = Split(command, ' ');
            if (args.empty())
                continue;

			std::unordered_set<std::string> options;
			for (size_t i = options.size() - 1; i != static_cast<size_t>(-1); --i) {
				auto& arg = args[i];
				if (arg.starts_with("--")) {
					options.emplace(std::move(args[i]));
					args.erase(args.begin() + static_cast<intmax_t>(i));
				}
			}

            if (args[0] == "e" || args[0] == "exit" || args[0] == "q" || args[0] == "quit") {
                running = false;
            } else if (args[0] == "wizard" && args.size() > 1) {
                if (args[1] == "-init") {
					if (!sorcerer->Initialize()) {
						std::cerr << "No feet, no sweets!" << std::endl;
						return EXIT_FAILURE;
					}
					logger->SetSeverity(sorcerer->GetConfig().logSeverity);
                    if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->Initialize();
					}
                } else if (args[1] == "-load") {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						if (packageManager->HasMissedPackages()) {
							packageManager->InstallMissedPackages();
							continue;
						}
					} else {
						std::cerr << "Package Manager is not initialized!" << std::endl;
						continue;
					}
					if (auto pluginManager = sorcerer->GetPluginManager().lock()) {
						if (pluginManager->IsInitialized()) {
							std::cerr << "Plugin manager already loaded." << std::endl;
						} else {
							pluginManager->Initialize();
							std::cerr << "Plugin manager was loaded." << std::endl;
						}
					}
                } else if (args[1] == "-unload") {
					if (auto pluginManager = sorcerer->GetPluginManager().lock()) {
						if (!pluginManager->IsInitialized()) {
							std::cerr << "Plugin manager already unloaded." << std::endl;
						} else {
							pluginManager->Terminate();
							std::cerr << "Plugin manager was unloaded." << std::endl;
						}
					}
                } else if (args[1] == "-term") {
                    sorcerer->Terminate();
                } else if (args[1] == "-exit") {
					running = false;
                } else if (args[1] == "-help") {
					printHelp();
                }  else if (args[1] == "-version") {
					constexpr const char* year = __DATE__ + 7;
					std::cout << R"(            .)" "\n";
					std::cout << R"(           /:\            Wizard v)" << sorcerer->GetVersion().ToString() << "\n";
					std::cout << R"(          /;:.\           )";
					std::cout << "Copyright (C) 2023-" << year << " Untrusted Modders Team\n";
					std::cout << R"(         //;:. \)" "\n";
					std::cout << R"(        ///;:.. \         This program may be freely redistributed under)" "\n";
					std::cout << R"(  __--"////;:... \"--__   the terms of the GNU General Public License.)" "\n";
					std::cout << R"(--__   "--_____--"   __--)" "\n";
					std::cout << R"(    """--_______--"""")" "\n";
					std::cout << std::endl;
                } else if (args[1] == "-module" && args.size() > 2) {
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
						if (!pluginManager->IsInitialized()) {
							std::cerr << "You must load plugin manager before query any information from it." << std::endl;
							continue;
						}
						for (auto& moduleRef : pluginManager->GetModules()) {
							auto& module = moduleRef.get();
							if (module.GetName() == args[2]) {
								PrintElement(module.GetDescriptor().language, numWidth, separator);
								PrintElement(module.GetName(), nameWidth, separator);
								PrintElement(module.GetFriendlyName(), fullNameWidth, separator);
								PrintElement(wizard::ModuleStateToString(module.GetState()), statusWidth, separator);
								std::cout << std::endl;
								break;
							}
						}
					}
				} else if (args[1] == "-plugin" && args.size() > 2) {
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
						if (!pluginManager->IsInitialized()) {
							std::cerr << "You must load plugin manager before query any information from it." << std::endl;
							continue;
						}
						for (auto& pluginRed : pluginManager->GetPlugins()) {
							auto& plugin = pluginRed.get();
							if (plugin.GetName() == args[2]) {
								PrintElement(plugin.GetId(), numWidth, separator);
								PrintElement(plugin.GetName(), nameWidth, separator);
								PrintElement(plugin.GetFriendlyName(), fullNameWidth, separator);
								PrintElement(wizard::PluginStateToString(plugin.GetState()), statusWidth, separator);
								std::cout << std::endl;
								break;
							}
						}
					}
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
						if (!pluginManager->IsInitialized()) {
							std::cerr << "You must load plugin manager before query any information from it." << std::endl;
							continue;
						}
                        for (auto& moduleRef : pluginManager->GetModules()) {
							auto& module = moduleRef.get();
                            PrintElement(module.GetDescriptor().language, numWidth, separator);
                            PrintElement(module.GetName(), nameWidth, separator);
                            PrintElement(module.GetFriendlyName(), fullNameWidth, separator);
                            PrintElement(wizard::ModuleStateToString(module.GetState()), statusWidth, separator);
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
						if (!pluginManager->IsInitialized()) {
							std::cerr << "You must load plugin manager before query any information from it." << std::endl;
							continue;
						}
                        for (auto& pluginRed : pluginManager->GetPlugins()) {
							auto& plugin = pluginRed.get();
                            PrintElement(plugin.GetId(), numWidth, separator);
                            PrintElement(plugin.GetName(), nameWidth, separator);
                            PrintElement(plugin.GetFriendlyName(), fullNameWidth, separator);
                            PrintElement(wizard::PluginStateToString(plugin.GetState()), statusWidth, separator);
                            std::cout << std::endl;
                        }
                    }
                } else if (args[1] == "-snapshot") {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						packageManager->SnapshotPackages(sorcerer->GetConfig().baseDir / std::format("snapshot_{}.wpackagemanifest", FormatTime("%Y_%m_%d_%H_%M_%S")), true);
					}
				} else if (args[1] == "-install" && (args.size() > 2 || !options.empty())) {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						if (options.contains("--link")) {
							packageManager->InstallAllPackages(args[2], args.size() > 3);
						} else if (options.contains("--file")) {
							packageManager->InstallAllPackages(std::filesystem::path{args[2]}, args.size() > 3);
						} else if (options.empty()) {
							packageManager->InstallPackages(std::span(args.begin() + 2, args.size() - 2));
						}
					}
				} else if (args[1] == "-uninstall" && (args.size() > 2 || !options.empty())) {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						if (options.contains("--all")) {
							packageManager->UninstallAllPackages();
						} else if (options.empty()) {
							packageManager->UninstallPackages(std::span(args.begin() + 2, args.size() - 2));
						}
					}
				}else if (args[1] == "-update" && (args.size() > 2 || !options.empty())) {
					if (auto packageManager = sorcerer->GetPackageManager().lock()) {
						if (options.contains("--all")) {
							packageManager->UpdateAllPackages();
						} else if (options.empty()) {
							packageManager->UpdatePackages(std::span(args.begin() + 2, args.size() - 2));
						}
					}
				} else if (args[1] == "-list") {
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
				} else if (args[1] == "-query") {
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

				} else if (args[1] == "-show" && args.size() > 2) {
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
								else {
									for (const auto& platform : package.descriptor->supportedPlatforms) {
										std::cout << "[" << platform << "] ";
									}
								}
								std::cout << std::endl;
								break;
							}
						}
					}
				} else if (args[1] == "-search" && args.size() > 2) {
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