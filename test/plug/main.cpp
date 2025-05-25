#include "plugify/plugify.hpp"
#include "std_logger.hpp"
#include <plugify/compat_format.hpp>
#include <plugify/date_time.hpp>
#include <plugify/language_module_descriptor.hpp>
#include <plugify/module.hpp>
#include <plugify/package.hpp>
#include <plugify/package_manager.hpp>
#include <plugify/plugin.hpp>
#include <plugify/plugin_descriptor.hpp>
#include <plugify/plugin_manager.hpp>
#include <plugify/plugin_reference_descriptor.hpp>
#include <plugify/debugging.hpp>

#include <client/crashpad_client.h>
#include <client/crash_report_database.h>
#include <client/settings.h>
#include <glaze/glaze.hpp>

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
		std::format_to(std::back_inserter(result), "[{:02d}] <{}> {}", t.GetId(), f(t.GetState()), t.GetFriendlyName());
	} else {
		std::format_to(std::back_inserter(result), "[{:02d}] {}", t.GetId(), t.GetFriendlyName());
	}
	auto descriptor = t.GetDescriptor();
	const auto& versionName = descriptor.GetVersionName();
	if (!versionName.empty()){
		std::format_to(std::back_inserter(result), " ({})", versionName);
	} else {
		std::format_to(std::back_inserter(result), " ({})", descriptor.GetVersion());
	}
	const auto& createdBy = descriptor.GetCreatedBy();
	if (!createdBy.empty()) {
		std::format_to(std::back_inserter(result), " by {}", createdBy);
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
	auto descriptor = t.GetDescriptor();
	const auto& getCreatedBy = descriptor.GetCreatedBy();
	if (!getCreatedBy.empty()) {
		PLG_LOG_FMT("  Name: \"{}\" by {}", t.GetFriendlyName(), getCreatedBy);
	} else {
		PLG_LOG_FMT("  Name: \"{}\"", t.GetFriendlyName());
	}
	const auto& versionName = descriptor.GetVersionName();
	if (!versionName.empty()) {
		PLG_LOG_FMT("  Version: {}", versionName);
	} else {
		PLG_LOG_FMT("  Version: {}", descriptor.GetVersion());
	}
	const auto& description = descriptor.GetDescription();
	if (!description.empty()) {
		PLG_LOG_FMT("  Description: {}", description);
	}
	const auto& createdByURL = descriptor.GetCreatedByURL();
	if (!createdByURL.empty()) {
		PLG_LOG_FMT("  URL: {}", createdByURL);
	}
	const auto& docsURL = descriptor.GetDocsURL();
	if (!docsURL.empty()) {
		PLG_LOG_FMT("  Docs: {}", docsURL);
	}
	const auto& downloadURL = descriptor.GetDownloadURL();
	if (!downloadURL.empty()) {
		PLG_LOG_FMT("  Download: {}", downloadURL);
	}
	const auto& updateURL = descriptor.GetUpdateURL();
	if (!updateURL.empty()) {
		PLG_LOG_FMT("  Update: {}", updateURL);
	}
}

using namespace plugify;
using namespace crashpad;

#if PLUGIFY_PLATFORM_APPLE
std::string GetExecutableDir() {
	std::string buffer(1024, '\0');
	uint32_t size = static_cast<uint32_t>(buffer.size());
	if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
		buffer.resize(size);
		if (_NSGetExecutablePath(buffer.data(), &size) != 0)
			throw std::runtime_error("Failed to get executable path (macOS)");
	}
	buffer.resize(strlen(buffer.c_str()));

	size_t lastSlash = buffer.find_last_of("/\\");
	return (lastSlash != std::string::npos) ? buffer.substr(0, lastSlash) : "";
}
#elif PLUGIFY_PLATFORM_LINUX
std::string GetExecutableDir() {
	std::string buffer(PATH_MAX, '\0');
	ssize_t count = readlink("/proc/self/exe", buffer.data(), buffer.size());
	if (count == -1)
		throw std::runtime_error("Failed to read /proc/self/exe");
	buffer.resize(count);

	size_t lastSlash = buffer.find_last_of("/\\");
	return (lastSlash != std::string::npos) ? buffer.substr(0, lastSlash) : "";
}
#elif PLUGIFY_PLATFORM_WINDOWS
std::wstring GetExecutableDir() {
	std::wstring buffer(MAX_PATH, L'\0');
	DWORD size = GetModuleFileNameW(NULL, buffer.data(), static_cast<DWORD>(buffer.size()));
	if (size == 0 || size == buffer.size())
		throw std::runtime_error("Failed to get executable path");
	buffer.resize(size);

	size_t lastSlash = buffer.find_last_of(L"\\/");
	return (lastSlash != std::wstring::npos) ? buffer.substr(0, lastSlash) : L"";
}
#endif

struct Metadata {
	std::string url;
	std::string handlerApp;
	std::string databaseDir;
	std::string metricsDir;
	std::map<std::string, std::string> annotations;
	std::vector<std::string> arguments;
	std::vector<std::string> attachments;
	std::optional<bool> restartable;
	std::optional<bool> asynchronous_start;
	std::optional<bool> enabled;
};

bool InitializeCrashpad(const std::filesystem::path& exeDir, const std::filesystem::path& annotationsPath) {
	auto json = ReadText(annotationsPath);
	auto metadata = glz::read_jsonc<Metadata>(json);
	if (!metadata.has_value()) {
		PLG_ERROR_FMT("Metadata: '{}' has JSON parsing error: {}", annotationsPath.string(), glz::format_error(metadata.error(), json));
		return false;
	}

	if (!metadata->enabled.value_or(true)) {
		return false;
	}

	base::FilePath handlerApp(exeDir / std::format(PLUGIFY_EXECUTABLE_PREFIX "{}" PLUGIFY_EXECUTABLE_SUFFIX, metadata->handlerApp));
	base::FilePath databaseDir(exeDir / metadata->databaseDir);
	base::FilePath metricsDir(exeDir / metadata->metricsDir);

	std::unique_ptr<CrashReportDatabase> database = CrashReportDatabase::Initialize(databaseDir);
	if (database == nullptr)
		return false;

	// File paths of attachments to uploaded with minidump file at crash time - default upload limit is 2MB
	std::vector<base::FilePath> attachments;
	attachments.reserve(metadata->attachments.size());
	for (const auto& attachment : metadata->attachments) {
		attachments.emplace_back(exeDir / attachment);
	}

	// Enable automated crash uploads
	Settings* settings = database->GetSettings();
	if (settings == nullptr)
		return false;
	settings->SetUploadsEnabled(true);

	// Start crash handler
	auto* client = new CrashpadClient();
	bool status = client->StartHandler(
		handlerApp,
		databaseDir,
		metricsDir,
		metadata->url,
		metadata->annotations,
		metadata->arguments,
		metadata->restartable.value_or(true),
		metadata->asynchronous_start.value_or(false),
		attachments);
	return status;
}

int main() {
	if (!plg::is_debugger_present()) {
		// Get directory where the exe lives so we can pass a full path to handler, reportsDir, metricsDir and attachments
		std::filesystem::path exeDir = GetExecutableDir();

		InitializeCrashpad(exeDir, "crashpad.jsonc");
	}
    std::shared_ptr<IPlugify> plug = MakePlugify();
    if (plug) {
        auto logger = std::make_shared<plug::StdLogger>();
        plug->SetLogger(logger);
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
					if (!plug->Initialize()) {
						PLG_ERROR("No feet, no sweets!");
						return 1;
					}
					logger->SetSeverity(plug->GetConfig().logSeverity.value_or(Severity::Debug));
                    if (auto packageManager = plug->GetPackageManager().lock()) {
						packageManager->Initialize();

						if (packageManager->HasMissedPackages()) {
							PLG_ERROR("Plugin manager has missing packages, run 'install --missing' to resolve issues.");
							continue;
						}
						if (packageManager->HasConflictedPackages()) {
							PLG_ERROR("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
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
						PLG_ERROR("Initialize system before use.");
						continue;
					}

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
						PLG_LOG("Package Manager commands:");
						PLG_LOG("  install <name> - Packages to install (space separated)");
						PLG_LOG("  remove <name>  - Packages to remove (space separated)");
						PLG_LOG("  update <name>  - Packages to update (space separated)");
						PLG_LOG("  list           - Print all local packages");
						PLG_LOG("  query          - Print all remote packages");
						PLG_LOG("  show  <name>   - Show information about local package");
						PLG_LOG("  search <name>  - Search information about remote package");
						PLG_LOG("  snapshot       - Snapshot packages into manifest");
						PLG_LOG("  repo <url>     - Add repository to config");
						PLG_LOG("Package Manager options:");
						PLG_LOG("  -h, --help     - Show help");
						PLG_LOG("  -a, --all      - Install/remove/update all packages");
						PLG_LOG("  -f, --file     - Packages to install (from file manifest)");
						PLG_LOG("  -l, --link     - Packages to install (from HTTP manifest)");
						PLG_LOG("  -m, --missing  - Install missing packages");
						PLG_LOG("  -c, --conflict - Remove conflict packages");
						PLG_LOG("  -i, --ignore   - Ignore missing or conflict packages");
					}

					else if (args[1] == "version" || args[1] == "-v") {
						static std::string copyright = std::format("Copyright (C) 2023-{}{}{}{} Untrusted Modders Team", __DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10]);
						PLG_LOG(R"(      ____)" "");
						PLG_LOG(R"( ____|    \         Plugify )" << plug->GetVersion());
						PLG_LOG(R"((____|     `._____  )" << copyright);
						PLG_LOG(R"( ____|       _|___)" "");
						PLG_LOG(R"((____|     .'       This program may be freely redistributed under)" "");
						PLG_LOG(R"(     |____/         the terms of the MIT License.)" "");
					}

					else if (args[1] == "load") {
						packageManager->Reload();
						if (!options.contains("--ignore") && !options.contains("-i")) {
							if (packageManager->HasMissedPackages()) {
								PLG_ERROR("Plugin manager has missing packages, run 'update --missing' to resolve issues.");
								continue;
							}
							if (packageManager->HasConflictedPackages()) {
								PLG_ERROR("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
								continue;
							}
						}
						if (pluginManager->IsInitialized()) {
							PLG_ERROR("Plugin manager already loaded.");
						} else {
							pluginManager->Initialize();
							PLG_LOG("Plugin manager was loaded.");
						}
					}

					else if (args[1] == "unload") {
						if (!pluginManager->IsInitialized()) {
							PLG_ERROR("Plugin manager already unloaded.");
						} else {
							pluginManager->Terminate();
							PLG_LOG("Plugin manager was unloaded.");
						}
						packageManager->Reload();
					}

					else if (args[1] == "reload") {
						if (!pluginManager->IsInitialized()) {
							PLG_ERROR("Plugin manager not loaded.");
							packageManager->Reload();
						} else {
							pluginManager->Terminate();
							packageManager->Reload();
							pluginManager->Initialize();
							PLG_LOG("Plugin manager was reloaded.");
						}
					}

					else if (args[1] == "plugins") {
						if (!pluginManager->IsInitialized()) {
							PLG_ERROR("You must load plugin manager before query any information from it.");
							continue;
						}
						auto plugins = pluginManager->GetPlugins();
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
						if (!pluginManager->IsInitialized()) {
							PLG_ERROR("You must load plugin manager before query any information from it.");
							continue;
						}
						auto modules = pluginManager->GetModules();
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

					else if (args[1] == "plugin") {
						if (args.size() > 2) {
							if (!pluginManager->IsInitialized()) {
								PLG_ERROR("You must load plugin manager before query any information from it.");
								continue;
							}
							auto plugin = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindPluginFromId(FormatInt(args[2])) : pluginManager->FindPlugin(args[2]);
							if (plugin) {
								Print<PluginState>("Plugin", plugin, PluginUtils::ToString);
								auto descriptor = plugin.GetDescriptor();
								PLG_LOG_FMT("  Language module: {}", descriptor.GetLanguageModule());
								PLG_LOG("  Dependencies: ");
								for (const auto& reference : descriptor.GetDependencies()) {
									auto dependency = pluginManager->FindPlugin(reference.GetName());
									if (dependency) {
										Print<PluginState>(dependency, PluginUtils::ToString, "    ");
									} else {
										auto version = reference.GetRequestedVersion();
										PLG_LOG_FMT("    {} <Missing> (v{})", reference.GetName(), version.has_value() ? version->to_string() : "[latest]");
									}
								}
								PLG_LOG_FMT("  File: {}", descriptor.GetEntryPoint());
							} else {
								PLG_ERROR_FMT("Plugin {} not found.", args[2]);
							}
						} else {
							PLG_ERROR("You must provide name.");
						}
					}

					else if (args[1] == "module") {
						if (args.size() > 2) {
							if (!pluginManager->IsInitialized()) {
								PLG_ERROR("You must load plugin manager before query any information from it.");
								continue;
							}
							auto module = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindModuleFromId(FormatInt(args[2])) : pluginManager->FindModule(args[2]);
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

					else if (args[1] == "snapshot") {
						if (pluginManager->IsInitialized()) {
							PLG_ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						packageManager->SnapshotPackages(plug->GetConfig().baseDir / std::format("snapshot_{}.wpackagemanifest", DateTime::Get("%Y_%m_%d_%H_%M_%S")), true);
					}

					else if (args[1] == "repo") {
						if (pluginManager->IsInitialized()) {
							PLG_ERROR("You must unload plugin manager before bring any change with package manager.");
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
							PLG_ERROR("You must give at least one repository to add.");
						}
					}

					else if (args[1] == "install") {
						if (pluginManager->IsInitialized()) {
							PLG_ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (options.contains("--missing") || options.contains("-m")) {
							if (packageManager->HasMissedPackages()) {
								packageManager->InstallMissedPackages();
							} else {
								PLG_LOG("No missing packages were found.");
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
								PLG_ERROR("You must give at least one requirement to install.");
							}
						}
					}

					else if (args[1] == "remove") {
						if (pluginManager->IsInitialized()) {
							PLG_ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (options.contains("--all") || options.contains("-a")) {
							packageManager->UninstallAllPackages();
						} else if (options.contains("--conflict") || options.contains("-c")) {
							if (packageManager->HasConflictedPackages()) {
								packageManager->UninstallConflictedPackages();
							} else {
								PLG_LOG("No conflicted packages were found.");
							}
						} else {
							if (args.size() > 2) {
								packageManager->UninstallPackages(std::span(args.begin() + 2, args.size() - 2));
							} else {
								PLG_ERROR("You must give at least one requirement to remove.");
							}
						}
					}

					else if (args[1] == "update") {
						if (pluginManager->IsInitialized()) {
							PLG_ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (options.contains("--all") || options.contains("-a")) {
							packageManager->UpdateAllPackages();
						} else {
							if (args.size() > 2) {
								packageManager->UpdatePackages(std::span(args.begin() + 2, args.size() - 2));
							} else {
								PLG_ERROR("You must give at least one requirement to update.");
							}
						}
					}

					else if (args[1] == "list") {
						if (pluginManager->IsInitialized()) {
							PLG_ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						auto localPackages = packageManager->GetLocalPackages();
						auto count = localPackages.size();
						if (!count) {
							PLG_ERROR("No local packages found.");
						} else {
							PLG_LOG_FMT("Listing {} local package{}:", count, (count > 1) ? "s" : "");
						}
						for (const auto& localPackage : localPackages) {
							PLG_LOG_FMT("  {} [{}] (v{}) at {}", localPackage->name, localPackage->type, localPackage->version, localPackage->path.string());
						}
					}

					else if (args[1] == "query") {
						if (pluginManager->IsInitialized()) {
							PLG_ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						auto remotePackages = packageManager->GetRemotePackages();
						auto count = remotePackages.size();
						if (!count) {
							PLG_ERROR("No remote packages found.");
						} else {
							PLG_LOG_FMT("Listing {} remote package{}:", count, (count > 1) ? "s" : "");
						}
						for (const auto& remotePackage : remotePackages) {
							if (remotePackage->author.empty() || remotePackage->description.empty()) {
								PLG_LOG_FMT("  {} [{}]", remotePackage->name, remotePackage->type);
							} else {
								PLG_LOG_FMT("  {} [{}] ({}) by {}", remotePackage->name, remotePackage->type, remotePackage->description, remotePackage->author);
							}
						}
					}

					else if (args[1] == "show") {
						if (pluginManager->IsInitialized()) {
							PLG_ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (args.size() > 2) {
							auto package = packageManager->FindLocalPackage(args[2]);
							if (package) {
								PLG_LOG_FMT("  Name: {}", package->name);
								PLG_LOG_FMT("  Type: {}", package->type);
								PLG_LOG_FMT("  Version: {}", package->version);
								PLG_LOG_FMT("  File: {}", package->path.string());

							} else {
								PLG_ERROR_FMT("Package {} not found.", args[2]);
							}
						} else {
							PLG_ERROR("You must provide name.");
						}
					}

					else if (args[1] == "search") {
						if (pluginManager->IsInitialized()) {
							PLG_ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (args.size() > 2) {
							auto package = packageManager->FindRemotePackage(args[2]);
							if (package) {
								PLG_LOG_FMT("  Name: {}", package->name);
								PLG_LOG_FMT("  Type: {}", package->type);
								if (!package->author.empty()) {
									PLG_LOG_FMT("  Author: {}", package->author);
								}
								if (!package->description.empty()) {
									PLG_LOG_FMT("  Description: {}", package->description);
								}
								const auto& versions = package->versions;
								if (!versions.empty()) {
									std::string combined("  Versions: ");
									std::format_to(std::back_inserter(combined), "{}", versions.begin()->version);
									for (auto it = std::next(versions.begin()); it != versions.end(); ++it) {
										std::format_to(std::back_inserter(combined), ", {}", it->version);
									}
									PLG_LOG(combined);
								} else {
									PLG_LOG("");
								}
							} else {
								PLG_ERROR_FMT("Package {} not found.", args[2]);
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

				plug->Update();
			} else {
				PLG_ERROR("usage: plg <command> [options] [arguments]");
				PLG_ERROR("Try plg help or -h for more information.");
			}
        }
    }

    return EXIT_SUCCESS;
}
