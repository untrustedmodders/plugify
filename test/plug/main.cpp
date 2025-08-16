
#include "std_logger.hpp"
#include <plg/format.hpp>
#include <plugify/api/date_time.hpp>
#include <plugify/api/dependency.hpp>
#include <plugify/api/manager.hpp>
#include <plugify/api/module.hpp>
#include <plugify/api/module_manifest.hpp>
#include <plugify/api/plugify.hpp>
#include <plugify/api/plugin.hpp>
#include <plugify/api/plugin_manifest.hpp>

#include <chrono>
#include <fstream>
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

#define PLG_LOG(x) std::cout << x << std::endl
#define PLG_ERROR(x) std::cerr << x << std::endl
#define PLG_LOG_FMT(...) std::cout << std::format(__VA_ARGS__) << std::endl
#define PLG_ERROR_FMT(...) std::cerr << std::format(__VA_ARGS__) << std::endl

std::string ReadText(const std::filesystem::path& filepath) {
	std::ifstream is(filepath, std::ios::binary);

	if (!is.is_open()) {
		PLG_ERROR_FMT("File: '{}' could not be opened", filepath.string());
		return {};
	}

	// Stop eating new lines in binary mode!!!
	is.unsetf(std::ios::skipws);

	return { std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{} };
}

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

int main() {
    PlugifyHandle plug = MakePlugify();
    if (plug) {
        auto logger = std::make_shared<plug::StdLogger>();
        plug.SetLogger(logger);
		logger->SetSeverity(Severity::Debug);
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
            } else if ((args[0] == "plg" || args[0] == "plugify") && args.size() > 1) {
                if (args[1] == "init") {
					if (!plug.Initialize()) {
						PLG_ERROR("No feet, no sweets!");
						return 1;
					}
					logger->SetSeverity(plug.GetConfig().logSeverity.value_or(Severity::Debug));

                	plug.GetManager().Initialize();
                } else if (args[1] == "term") {
                    plug.Terminate();
                } else if (args[1] == "exit") {
					running = false;
                } else {
					if (!plug.IsInitialized()) {
						PLG_ERROR("Initialize system before use.");
						continue;
					}

                	auto manager = plug.GetManager();

					if (args[1] == "help" || args[1] == "-h") {
						PLG_LOG("Plugify Menu");
						PLG_LOG("(c) untrustedmodders");
						PLG_LOG("https://github.com/untrustedmodders");
						PLG_LOG("usage: plg <command> [options] [arguments]");
						PLG_LOG("  help           - Show help");
						PLG_LOG("  version        - Version information");
						PLG_LOG("Plugin Manager commands:");
						PLG_LOG("  load           - Load plugin manager");
						PLG_LOG("  unload         - Unload plugin manager");
						PLG_LOG("  reload         - Reload plugin manager");
						PLG_LOG("  modules        - List running modules");
						PLG_LOG("  plugins        - List running plugins");
						PLG_LOG("  plugin <name>  - Show information about a module");
						PLG_LOG("  module <name>  - Show information about a plugin");
						PLG_LOG("Plugin Manager options:");
						PLG_LOG("  -h, --help     - Show help");
						PLG_LOG("  -u, --uuid     - Use index instead of name");
					}

					else if (args[1] == "version" || args[1] == "-v") {
						static std::string copyright = std::format("Copyright (C) 2023-{}{}{}{} Untrusted Modders Team", __DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10]);
						PLG_LOG(R"(      ____)" "");
						PLG_LOG(R"( ____|    \         Plugify )" << plug.GetVersion());
						PLG_LOG(R"((____|     `._____  )" << copyright);
						PLG_LOG(R"( ____|       _|___)" "");
						PLG_LOG(R"((____|     .'       This program may be freely redistributed under)" "");
						PLG_LOG(R"(     |____/         the terms of the MIT License.)" "");
					}

					else if (args[1] == "load") {
						if (manager.IsInitialized()) {
							PLG_ERROR("Plugin manager already loaded.");
						} else {
							manager.Initialize();
							PLG_LOG("Plugin manager was loaded.");
						}
					}

					else if (args[1] == "unload") {
						if (!manager.IsInitialized()) {
							PLG_ERROR("Plugin manager already unloaded.");
						} else {
							manager.Terminate();
							PLG_LOG("Plugin manager was unloaded.");
						}
					}

					else if (args[1] == "reload") {
						if (!manager.IsInitialized()) {
							PLG_ERROR("Plugin manager not loaded.");
						} else {
							manager.Terminate();
							manager.Initialize();
							PLG_LOG("Plugin manager was reloaded.");
						}
					}

					else if (args[1] == "plugins") {
						if (!manager.IsInitialized()) {
							PLG_ERROR("You must load plugin manager before query any information from it.");
							continue;
						}
						auto plugins = manager.GetPlugins();
						auto count = plugins.size();
						if (!count) {
							PLG_ERROR("No plugins loaded.");
						} else {
							PLG_LOG_FMT("Listing {} plugin{}:", count, (count > 1) ? "s" : "");
						}
						for (const auto& plugin : plugins) {
							Print<PluginState>(plugin, PluginUtils::ToString);
						}
					}

					else if (args[1] == "modules") {
						if (!manager.IsInitialized()) {
							PLG_ERROR("You must load plugin manager before query any information from it.");
							continue;
						}
						auto modules = manager.GetModules();
						auto count = modules.size();
						if (!count) {
							PLG_ERROR("No modules loaded.");
						} else {
							PLG_LOG_FMT("Listing {} module{}:", count, (count > 1) ? "s" : "");
						}
						for (const auto& module : modules) {
							Print<ModuleState>(module, ModuleUtils::ToString);
						}
					}

					else if (args[1] == "plugins") {
						if (args.size() > 2) {
							if (!manager.IsInitialized()) {
								PLG_ERROR("You must load plugin manager before query any information from it.");
								continue;
							}
							auto plugin = options.contains("--uuid") || options.contains("-u") ? manager.FindPluginFromId(FormatInt(args[2])) : manager.FindPlugin(args[2]);
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
											PLG_LOG_FMT("    {} <Missing> ({}-v{})", reference.GetName(), static_cast<int>(type), version.to_string());
										}
									}
								}
								PLG_LOG_FMT("  File: {}", manifest.GetEntry());
							} else {
								PLG_ERROR_FMT("Plugin {} not found.", args[2]);
							}
						} else {
							PLG_ERROR("You must provide name.");
						}
					}

					else if (args[1] == "module") {
						if (args.size() > 2) {
							if (!manager.IsInitialized()) {
								PLG_ERROR("You must load plugin manager before query any information from it.");
								continue;
							}
							auto module = options.contains("--uuid") || options.contains("-u") ? manager.FindModuleFromId(FormatInt(args[2])) : manager.FindModule(args[2]);
							if (module) {
								Print<ModuleState>("Module", module, ModuleUtils::ToString);
								PLG_LOG_FMT("  Language: {}", module.GetLanguage());
								PLG_LOG_FMT("  File: {}", std::filesystem::path(module.GetFilePath()).string());
							} else {
								PLG_ERROR_FMT("Module {} not found.", args[2]);
							}
						} else {
							PLG_ERROR("You must provide name.");
						}
					}

					else {
						PLG_ERROR_FMT("unknown option: {}", args[1]);
						PLG_ERROR("usage: plg <command> [options] [arguments]");
						PLG_ERROR("Try plg help or -h for more information.");
					}
				}

				plug.Update();
			} else {
				PLG_ERROR("usage: plg <command> [options] [arguments]");
				PLG_ERROR("Try plg help or -h for more information.");
			}
        }
    }

    return EXIT_SUCCESS;
}
