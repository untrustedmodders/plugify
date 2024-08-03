#include "std_logger.h"
#include <plugify/compat_format.h>
#include <plugify/plugify.h>
#include <plugify/module.h>
#include <plugify/package.h>
#include <plugify/package_manager.h>
#include <plugify/plugin_manager.h>
#include <plugify/plugin.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/plugin_reference_descriptor.h>
#include <plugify/language_module_descriptor.h>

#include <chrono>
#include <iostream>
#include <unordered_set>

std::vector<std::string> Split(const std::string& str, char sep) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream is(str);

    while (std::getline(is, token, sep))
        tokens.emplace_back(token);
    return tokens;
}

std::string FormatTime(std::string_view format = "%Y-%m-%d %H:%M:%S") {
	auto now = std::chrono::system_clock::now();
	auto timeT = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&timeT), format.data());
	return ss.str();
}

#define CONPRINT(x) std::cout << x << std::endl
#define CONPRINTE(x) std::cerr << x << std::endl
#define CONPRINTF(...) std::cout << std::format(__VA_ARGS__) << std::endl

ptrdiff_t FormatInt(const std::string& str) {
	try {
		size_t pos;
		ptrdiff_t result = std::stoll(str, &pos);
		if (pos != str.length()) {
			throw std::invalid_argument("Trailing characters after the valid part");
		}
		return result;
	} catch (const std::invalid_argument& e) {
		CONPRINTF("Invalid argument: {}", e.what());
	} catch (const std::out_of_range& e) {
		CONPRINTF("Out of range: {}", e.what());
	} catch (const std::exception& e) {
		CONPRINTF("Conversion error: {}", e.what());
	}

	return -1;
}

template<typename S, typename T, typename F>
void Print(T t, F&& f, std::string_view tab = "  ") {
	std::string result(tab);
	if (t.GetState() != S::Loaded) {
		std::format_to(std::back_inserter(result), "[{:02d}] <{}> {}", t.GetId(), f(t.GetState()), t.GetFriendlyName());
	} else {
		std::format_to(std::back_inserter(result), "[{:02d}] {}", t.GetId(), t.GetFriendlyName());
	}
	auto descriptor = t.GetDescriptor();
	const auto& versionName = descriptor.GetVersionName();
	if (!versionName.empty()){
		std::format_to(std::back_inserter(result), " ({})", versionName);
	} else {
		std::format_to(std::back_inserter(result), " (v{})", descriptor.GetVersion());
	}
	const auto& createdBy = descriptor.GetCreatedBy();
	if (!createdBy.empty()) {
		std::format_to(std::back_inserter(result), " by {}", createdBy);
	}
	CONPRINT(result);
}

template<typename S, typename T, typename F>
void Print(std::string_view name, T t, F&& f) {
	if (t.GetState() == S::Error) {
		CONPRINTF("{} has error: {}.", name, t.GetError());
	} else {
		CONPRINTF("{} {} is {}.", name, t.GetId(), f(t.GetState()));
	}
	auto descriptor = t.GetDescriptor();
	const auto& getCreatedBy = descriptor.GetCreatedBy();
	if (!getCreatedBy.empty()) {
		CONPRINTF("  Name: \"{}\" by {}", t.GetFriendlyName(), getCreatedBy);
	} else {
		CONPRINTF("  Name: \"{}\"", t.GetFriendlyName());
	}
	const auto& versionName = descriptor.GetVersionName();
	if (!versionName.empty()) {
		CONPRINTF("  Version: {}", versionName);
	} else {
		CONPRINTF("  Version: {}", descriptor.GetVersion());
	}
	const auto& description = descriptor.GetDescription();
	if (!description.empty()) {
		CONPRINTF("  Description: {}", description);
	}
	const auto& createdByURL = descriptor.GetCreatedByURL();
	if (!createdByURL.empty()) {
		CONPRINTF("  URL: {}", createdByURL);
	}
	const auto& docsURL = descriptor.GetDocsURL();
	if (!docsURL.empty()) {
		CONPRINTF("  Docs: {}", docsURL);
	}
	const auto& downloadURL = descriptor.GetDownloadURL();
	if (!downloadURL.empty()) {
		CONPRINTF("  Download: {}", downloadURL);
	}
	const auto& updateURL = descriptor.GetUpdateURL();
	if (!updateURL.empty()) {
		CONPRINTF("  Update: {}", updateURL);
	}
}

int main() {
    std::shared_ptr<plugify::IPlugify> plug = plugify::MakePlugify();
    if (plug) {
        auto logger = std::make_shared<plug::StdLogger>();
        plug->SetLogger(logger);
		logger->SetSeverity(plugify::Severity::Debug);
        bool running = true;
        while (running) {
            std::string command;
            std::getline(std::cin, command);
            std::vector<std::string> args = Split(command, ' ');
            if (args.empty())
                continue;

			std::unordered_set<std::string> options;
			for (size_t i = args.size() - 1; i != static_cast<size_t>(-1); --i) {
				auto& arg = args[i];
				if (i > 1 && arg.starts_with("-")) {
					options.emplace(std::move(args[i]));
					args.erase(args.begin() + static_cast<ptrdiff_t>(i));
				}
			}

            if (args[0] == "-e" || args[0] == "exit" || args[0] == "-q" || args[0] == "quit") {
                running = false;
            } else if (args[0] == "plugify" && args.size() > 1) {
                if (args[1] == "init") {
					if (!plug->Initialize()) {
						CONPRINTE("No feet, no sweets!");
						return EXIT_FAILURE;
					}
					logger->SetSeverity(plug->GetConfig().logSeverity);
                    if (auto packageManager = plug->GetPackageManager().lock()) {
						packageManager->Initialize();

						if (packageManager->HasMissedPackages()) {
							CONPRINTE("Plugin manager has missing packages, run 'install --missing' to resolve issues.");
							continue;
						}
						if (packageManager->HasConflictedPackages()) {
							CONPRINTE("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
							continue;
						}
					}

					if (auto pluginManager = plug->GetPluginManager().lock()) {
						pluginManager->Initialize();
					}
                } else if (args[1] == "term") {
                    plug->Terminate();
                } else if (args[1] == "exit") {
					running = false;
                } else {
					auto packageManager = plug->GetPackageManager().lock();
					auto pluginManager = plug->GetPluginManager().lock();
					if (!packageManager || !pluginManager) {
						CONPRINTE("Initialize system before use.");
						continue;
					}

					if (args[1] == "help" || args[1] == "-h") {
						CONPRINT("Plugify Menu");
						CONPRINT("(c) untrustedmodders");
						CONPRINT("https://github.com/untrustedmodders");
						CONPRINT("usage: plugify <command> [options] [arguments]");
						CONPRINT("  help           - Show help");
						CONPRINT("  version        - Version information");
						CONPRINT("Plugin Manager commands:");
						CONPRINT("  load           - Load plugin manager");
						CONPRINT("  unload         - Unload plugin manager");
						CONPRINT("  modules        - List running modules");
						CONPRINT("  plugins        - List running plugins");
						CONPRINT("  plugin <name>  - Show information about a module");
						CONPRINT("  module <name>  - Show information about a plugin");
						CONPRINT("Plugin Manager options:");
						CONPRINT("  -h, --help     - Show help");
						CONPRINT("  -u, --uuid     - Use index instead of name");
						CONPRINT("Package Manager commands:");
						CONPRINT("  install <name> - Packages to install (space separated)");
						CONPRINT("  remove <name>  - Packages to remove (space separated)");
						CONPRINT("  update <name>  - Packages to update (space separated)");
						CONPRINT("  list           - Print all local packages");
						CONPRINT("  query          - Print all remote packages");
						CONPRINT("  show  <name>   - Show information about local package");
						CONPRINT("  search <name>  - Search information about remote package");
						CONPRINT("  snapshot       - Snapshot packages into manifest");
						CONPRINT("  repo <url>     - Add repository to config");
						CONPRINT("Package Manager options:");
						CONPRINT("  -h, --help     - Show help");
						CONPRINT("  -a, --all      - Install/remove/update all packages");
						CONPRINT("  -f, --file     - Packages to install (from file manifest)");
						CONPRINT("  -l, --link     - Packages to install (from HTTP manifest)");
						CONPRINT("  -m, --missing  - Install missing packages");
						CONPRINT("  -c, --conflict - Remove conflict packages");
						CONPRINT("  -i, --ignore   - Ignore missing or conflict packages");
					}

					else if (args[1] == "version" || args[1] == "-v") {
						static std::string copyright = std::format("Copyright (C) 2023-{}{}{}{} Untrusted Modders Team", __DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10]);
						CONPRINT(R"(      ____)" "");
						CONPRINT(R"( ____|    \         Plugify v)" << plug->GetVersion().ToString());
						CONPRINT(R"((____|     `._____  )" << copyright);
						CONPRINT(R"( ____|       _|___)" "");
						CONPRINT(R"((____|     .'       This program may be freely redistributed under)" "");
						CONPRINT(R"(     |____/         the terms of the MIT License.)" "");
					}

					else if (args[1] == "load") {
						if (!options.contains("--ignore") && !options.contains("-i")) {
							if (packageManager->HasMissedPackages()) {
								CONPRINT("Plugin manager has missing packages, run 'update --missing' to resolve issues.");
								continue;
							}
							if (packageManager->HasConflictedPackages()) {
								CONPRINT("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
								continue;
							}
						}
						if (pluginManager->IsInitialized()) {
							CONPRINT("Plugin manager already loaded.");
						} else {
							pluginManager->Initialize();
							CONPRINT("Plugin manager was loaded.");
						}
					}

					else if (args[1] == "unload") {
						if (!pluginManager->IsInitialized()) {
							CONPRINT("Plugin manager already unloaded.");
						} else {
							pluginManager->Terminate();
							CONPRINT("Plugin manager was unloaded.");
						}
					}

					else if (args[1] == "plugins") {
						if (!pluginManager->IsInitialized()) {
							CONPRINT("You must load plugin manager before query any information from it.");
							continue;
						}
						auto count = pluginManager->GetPlugins().size();
						if (!count) {
							CONPRINT("No plugins loaded.");
						} else {
							CONPRINTF("Listing {} plugin{}:", static_cast<int>(count), (count > 1) ? "s" : "");
						}
						for (auto& plugin : pluginManager->GetPlugins()) {
							Print<plugify::PluginState>(plugin, plugify::PluginUtils::ToString);
						}
					}

					else if (args[1] == "modules") {
						if (!pluginManager->IsInitialized()) {
							CONPRINT("You must load plugin manager before query any information from it.");
							continue;
						}
						auto count = pluginManager->GetModules().size();
						if (!count) {
							CONPRINT("No modules loaded.");
						} else {
							CONPRINTF("Listing {} module{}:", static_cast<int>(count), (count > 1) ? "s" : "");
						}
						for (auto& module : pluginManager->GetModules()) {
							Print<plugify::ModuleState>(module, plugify::ModuleUtils::ToString);
						}
					}

					else if (args[1] == "plugin") {
						if (args.size() > 2) {
							if (!pluginManager->IsInitialized()) {
								CONPRINT("You must load plugin manager before query any information from it.");
								continue;
							}
							auto plugin = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindPluginFromId(FormatInt(args[2])) : pluginManager->FindPlugin(args[2]);
							if (plugin.has_value()) {
								Print<plugify::PluginState>("Plugin", *plugin, plugify::PluginUtils::ToString);
								auto descriptor = plugin->GetDescriptor();
								CONPRINTF("  Language module: {}", descriptor.GetLanguageModule());
								CONPRINT("  Dependencies: ");
								for (const auto& reference : descriptor.GetDependencies()) {
									auto dependency = pluginManager->FindPlugin(reference.GetName());
									if (dependency.has_value()) {
										Print<plugify::PluginState>(*dependency, plugify::PluginUtils::ToString, "    ");
									} else {
										auto version = reference.GetRequestedVersion();
										CONPRINTF("    {} <Missing> (v{})", reference.GetName(), version.has_value() ? std::to_string(*version) : "[latest]");
									}
								}
								CONPRINTF("  File: {}", descriptor.GetEntryPoint());
							} else {
								CONPRINTF("Plugin {} not found.", args[2]);
							}
						} else {
							CONPRINT("You must provide name.");
						}
					}

					else if (args[1] == "module") {
						if (args.size() > 2) {
							if (!pluginManager->IsInitialized()) {
								CONPRINT("You must load plugin manager before query any information from it.");
								continue;
							}
							auto module = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindModuleFromId(FormatInt(args[2])) : pluginManager->FindModule(args[2]);
							if (module.has_value()) {
								Print<plugify::ModuleState>("Module", *module, plugify::ModuleUtils::ToString);
								CONPRINTF("  Language: {}", module->GetLanguage());
								CONPRINTF("  File: {}", std::filesystem::path(module->GetFilePath()).string());
							} else {
								CONPRINTF("Module {} not found.", args[2]);
							}
						} else {
							CONPRINT("You must provide name.");
						}
					}

					else if (args[1] == "snapshot") {
						if (pluginManager->IsInitialized()) {
							CONPRINT("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						packageManager->SnapshotPackages(plug->GetConfig().baseDir / std::format("snapshot_{}.wpackagemanifest", FormatTime("%Y_%m_%d_%H_%M_%S")), true);
					}

					else if (args[1] == "repo") {
						if (pluginManager->IsInitialized()) {
							CONPRINT("You must unload plugin manager before bring any change with package manager.");
							continue;
						}

						if (args.size() > 2) {
							bool success = false;
							for (const auto& repository : std::span(args.begin() + 2, args.size() - 2)) {
								success |= plug->AddRepository(repository);
							}
							if (success) {
								packageManager->Reload();
							}
						} else {
							CONPRINT("You must give at least one repository to add.");
						}
					}

					else if (args[1] == "install") {
						if (pluginManager->IsInitialized()) {
							CONPRINT("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (options.contains("--missing") || options.contains("-m")) {
							if (packageManager->HasMissedPackages()) {
								packageManager->InstallMissedPackages();
							} else {
								CONPRINT("No missing packages were found.");
							}
						} else {
							if (args.size() > 2) {
								if (options.contains("--link") || options.contains("-l")) {
									packageManager->InstallAllPackages(args[2], args.size() > 3);
								} else if (options.contains("--file") || options.contains("-f")) {
									packageManager->InstallAllPackages(std::filesystem::path{args[2]}, args.size() > 3);
								} else {
									packageManager->InstallPackages(std::span(args.begin() + 2, args.size() - 2));
								}
							} else {
								CONPRINT("You must give at least one requirement to install.");
							}
						}
					}

					else if (args[1] == "remove") {
						if (pluginManager->IsInitialized()) {
							CONPRINT("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (options.contains("--all") || options.contains("-a")) {
							packageManager->UninstallAllPackages();
						} else if (options.contains("--conflict") || options.contains("-c")) {
							if (packageManager->HasConflictedPackages()) {
								packageManager->UninstallConflictedPackages();
							} else {
								CONPRINT("No conflicted packages were found.");
							}
						} else {
							if (args.size() > 2) {
								packageManager->UninstallPackages(std::span(args.begin() + 2, args.size() - 2));
							} else {
								CONPRINT("You must give at least one requirement to remove.");
							}
						}
					}

					else if (args[1] == "update") {
						if (pluginManager->IsInitialized()) {
							CONPRINT("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (options.contains("--all") || options.contains("-a")) {
							packageManager->UpdateAllPackages();
						} else {
							if (args.size() > 2) {
								packageManager->UpdatePackages(std::span(args.begin() + 2, args.size() - 2));
							} else {
								CONPRINT("You must give at least one requirement to update.");
							}
						}
					}

					else if (args[1] == "list") {
						if (pluginManager->IsInitialized()) {
							CONPRINT("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						auto count = packageManager->GetLocalPackages().size();
						if (!count) {
							CONPRINT("No local packages found.");
						} else {
							CONPRINTF("Listing {} local package{}:", static_cast<int>(count), (count > 1) ? "s" : "");
						}
						for (const auto& localPackage : packageManager->GetLocalPackages()) {
							CONPRINTF("  {} [{}] (v{}) at {}", localPackage.name, localPackage.type, localPackage.version, localPackage.path.string());
						}
					}

					else if (args[1] == "query") {
						if (pluginManager->IsInitialized()) {
							CONPRINT("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						auto count = packageManager->GetRemotePackages().size();
						if (!count) {
							CONPRINT("No remote packages found.");
						} else {
							CONPRINTF("Listing {} remote package{}:", static_cast<int>(count), (count > 1) ? "s" : "");
						}
						for (const auto& remotePackage : packageManager->GetRemotePackages()) {
							if (remotePackage.author.empty() || remotePackage.description.empty()) {
								CONPRINTF("  {} [{}]", remotePackage.name, remotePackage.type);
							} else {
								CONPRINTF("  {} [{}] ({}) by {}", remotePackage.name, remotePackage.type, remotePackage.description, remotePackage.author);
							}
						}
					}

					else if (args[1] == "show") {
						if (pluginManager->IsInitialized()) {
							CONPRINT("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (args.size() > 2) {
							auto package = packageManager->FindLocalPackage(args[2]);
							if (package.has_value()) {
								CONPRINTF("  Name: {}", package->name);
								CONPRINTF("  Type: {}", package->type);
								CONPRINTF("  Version: {}", package->version);
								CONPRINTF("  File: {}", package->path.string());

							} else {
								CONPRINTF("Package {} not found.", args[2]);
							}
						} else {
							CONPRINT("You must provide name.");
						}
					}

					else if (args[1] == "search") {
						if (pluginManager->IsInitialized()) {
							CONPRINT("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (args.size() > 2) {
							auto package = packageManager->FindRemotePackage(args[2]);
							if (package.has_value()) {
								CONPRINTF("  Name: {}", package->name);
								CONPRINTF("  Type: {}", package->type);
								if (!package->author.empty()) {
									CONPRINTF("  Author: {}", package->author);
								}
								if (!package->description.empty()) {
									CONPRINTF("  Description: {}", package->description);
								}
								if (!package->versions.empty()) {
									std::string versions("  Versions: ");
									std::format_to(std::back_inserter(versions), "{}", package->versions.begin()->version);
									for (auto it = std::next(package->versions.begin()); it != package->versions.end(); ++it) {
										std::format_to(std::back_inserter(versions), ", {}", it->version);
									}
									CONPRINT(versions);
								} else {
									CONPRINT("");
								}
							} else {
								CONPRINTF("Package {} not found.", args[2]);
							}
						} else {
							CONPRINT("You must provide name.");
						}
					}

					else {
						CONPRINTF("unknown option: {}", args[1]);
						CONPRINT("usage: plugify <command> [options] [arguments]");
						CONPRINT("Try plugify help or -h for more information.");
					}
				}
			} else {
				CONPRINT("usage: plugify <command> [options] [arguments]");
				CONPRINT("Try plugify help or -h for more information.");
			}
        }
    }

    return EXIT_SUCCESS;
}