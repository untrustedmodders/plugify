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

#define MESSAGE(x) std::cout << x << std::endl
#define ERROR(x) std::cerr << x << std::endl
#define MESSAGE_FMT(...) std::cout << std::format(__VA_ARGS__) << std::endl
#define ERROR_FMT(...) std::cerr << std::format(__VA_ARGS__) << std::endl

std::string ReadText(const std::filesystem::path& filepath) {
	std::ifstream is(filepath, std::ios::binary);

	if (!is.is_open()) {
		ERROR_FMT("File: '{}' could not be opened", filepath.string());
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
		ERROR_FMT("Error: {}", std::make_error_code(ec).message());
		return -1;
	} else if (ptr != str.data() + str.size()) {
		ERROR("Invalid argument: trailing characters after the valid part");
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
	MESSAGE(result);
}

template<typename S, typename T, typename F>
void Print(std::string_view name, T t, F&& f) {
	if (t.GetState() == S::Error) {
		ERROR_FMT("{} has error: {}.", name, t.GetError());
	} else {
		MESSAGE_FMT("{} {} is {}.", name, t.GetId(), f(t.GetState()));
	}
	auto descriptor = t.GetDescriptor();
	const auto& getCreatedBy = descriptor.GetCreatedBy();
	if (!getCreatedBy.empty()) {
		MESSAGE_FMT("  Name: \"{}\" by {}", t.GetFriendlyName(), getCreatedBy);
	} else {
		MESSAGE_FMT("  Name: \"{}\"", t.GetFriendlyName());
	}
	const auto& versionName = descriptor.GetVersionName();
	if (!versionName.empty()) {
		MESSAGE_FMT("  Version: {}", versionName);
	} else {
		MESSAGE_FMT("  Version: {}", descriptor.GetVersion());
	}
	const auto& description = descriptor.GetDescription();
	if (!description.empty()) {
		MESSAGE_FMT("  Description: {}", description);
	}
	const auto& createdByURL = descriptor.GetCreatedByURL();
	if (!createdByURL.empty()) {
		MESSAGE_FMT("  URL: {}", createdByURL);
	}
	const auto& docsURL = descriptor.GetDocsURL();
	if (!docsURL.empty()) {
		MESSAGE_FMT("  Docs: {}", docsURL);
	}
	const auto& downloadURL = descriptor.GetDownloadURL();
	if (!downloadURL.empty()) {
		MESSAGE_FMT("  Download: {}", downloadURL);
	}
	const auto& updateURL = descriptor.GetUpdateURL();
	if (!updateURL.empty()) {
		MESSAGE_FMT("  Update: {}", updateURL);
	}
}

using namespace plugify;
using namespace crashpad;

#if PLUGIFY_PLATFORM_APPLE
typedef std::string StringType;
#define STR(s) s

std::string getExecutableDir() {
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
typedef std::string StringType;
#define STR(s) s

std::string getExecutableDir() {
	std::string buffer(PATH_MAX, '\0');
	ssize_t count = readlink("/proc/self/exe", buffer.data(), buffer.size());
	if (count == -1)
		throw std::runtime_error("Failed to read /proc/self/exe");
	buffer.resize(count);

	size_t lastSlash = buffer.find_last_of("/\\");
	return (lastSlash != std::string::npos) ? buffer.substr(0, lastSlash) : "";
}
#elif PLUGIFY_PLATFORM_WINDOWS
typedef std::wstring StringType;
#define STR(s) L##s

std::wstring getExecutableDir() {
	StringType buffer(MAX_PATH, L'\0');
	DWORD size = GetModuleFileNameW(NULL, buffer.data(), static_cast<DWORD>(buffer.size()));
	if (size == 0 || size == buffer.size())
		throw std::runtime_error("Failed to get executable path");
	buffer.resize(size);

	size_t lastSlash = buffer.find_last_of(L"\\/");
	return (lastSlash != std::wstring::npos) ? buffer.substr(0, lastSlash) : L"";
}
#endif

struct Metadata {
	StringType url;
	StringType handlerApp;
	StringType databaseDir;
	StringType metricsDir;
	std::map<StringType, StringType> annotations;
	std::vector<StringType> arguments;
	std::vector<StringType> attachments;
	std::optional<bool> restartable;
	std::optional<bool> asynchronous_start;
	std::optional<bool> enabled;
};

bool initializeCrashpad(const std::filesystem::path& annotationsPath) {
	auto json = ReadText(annotationsPath);
	auto metadata = glz::read_jsonc<Metadata>(json);
	if (!metadata.has_value()) {
		ERROR_FMT("Metadata: '{}' has JSON parsing error: {}", annotationsPath.string(), glz::format_error(metadata.error(), json));
		return false;
	}

	if (!metadata->enabled.value_or(true)) {
		return false;
	}

	// Get directory where the exe lives so we can pass a full path to handler, reportsDir, metricsDir and attachments
	StringType exeDir = getExecutableDir();

	base::FilePath handlerApp(std::format(STR(PLUGIFY_EXECUTABLE_PREFIX) STR("{}/{}") STR(PLUGIFY_EXECUTABLE_SUFFIX), exeDir, metadata->handlerApp));
	base::FilePath databaseDir(std::format(STR("{}/{}"), exeDir, metadata->databaseDir));
	base::FilePath metricsDir(std::format(STR("{}/{}"), exeDir, metadata->metricsDir));

	std::unique_ptr<CrashReportDatabase> database = CrashReportDatabase::Initialize(databaseDir);
	if (database == nullptr)
		return false;

	// File paths of attachments to uploaded with minidump file at crash time - default upload limit is 2MB
	std::vector<base::FilePath> attachments;
	attachments.reserve(metadata->attachments.size());
	for (const auto& attachment : metadata->attachments) {
		attachments.emplace_back(std::format(STR("{}/{}"), exeDir, attachment));
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
		initializeCrashpad("crashpad.jsonc");
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
						ERROR("No feet, no sweets!");
						return 1;
					}
					logger->SetSeverity(plug->GetConfig().logSeverity.value_or(Severity::Debug));
                    if (auto packageManager = plug->GetPackageManager().lock()) {
						packageManager->Initialize();

						if (packageManager->HasMissedPackages()) {
							ERROR("Plugin manager has missing packages, run 'install --missing' to resolve issues.");
							continue;
						}
						if (packageManager->HasConflictedPackages()) {
							ERROR("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
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
						ERROR("Initialize system before use.");
						continue;
					}

					if (args[1] == "help" || args[1] == "-h") {
						MESSAGE("Plugify Menu");
						MESSAGE("(c) untrustedmodders");
						MESSAGE("https://github.com/untrustedmodders");
						MESSAGE("usage: plg <command> [options] [arguments]");
						MESSAGE("  help           - Show help");
						MESSAGE("  version        - Version information");
						MESSAGE("Plugin Manager commands:");
						MESSAGE("  load           - Load plugin manager");
						MESSAGE("  unload         - Unload plugin manager");
						MESSAGE("  reload         - Reload plugin manager");
						MESSAGE("  modules        - List running modules");
						MESSAGE("  plugins        - List running plugins");
						MESSAGE("  plugin <name>  - Show information about a module");
						MESSAGE("  module <name>  - Show information about a plugin");
						MESSAGE("Plugin Manager options:");
						MESSAGE("  -h, --help     - Show help");
						MESSAGE("  -u, --uuid     - Use index instead of name");
						MESSAGE("Package Manager commands:");
						MESSAGE("  install <name> - Packages to install (space separated)");
						MESSAGE("  remove <name>  - Packages to remove (space separated)");
						MESSAGE("  update <name>  - Packages to update (space separated)");
						MESSAGE("  list           - Print all local packages");
						MESSAGE("  query          - Print all remote packages");
						MESSAGE("  show  <name>   - Show information about local package");
						MESSAGE("  search <name>  - Search information about remote package");
						MESSAGE("  snapshot       - Snapshot packages into manifest");
						MESSAGE("  repo <url>     - Add repository to config");
						MESSAGE("Package Manager options:");
						MESSAGE("  -h, --help     - Show help");
						MESSAGE("  -a, --all      - Install/remove/update all packages");
						MESSAGE("  -f, --file     - Packages to install (from file manifest)");
						MESSAGE("  -l, --link     - Packages to install (from HTTP manifest)");
						MESSAGE("  -m, --missing  - Install missing packages");
						MESSAGE("  -c, --conflict - Remove conflict packages");
						MESSAGE("  -i, --ignore   - Ignore missing or conflict packages");
					}

					else if (args[1] == "version" || args[1] == "-v") {
						static std::string copyright = std::format("Copyright (C) 2023-{}{}{}{} Untrusted Modders Team", __DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10]);
						MESSAGE(R"(      ____)" "");
						MESSAGE(R"( ____|    \         Plugify )" << plug->GetVersion());
						MESSAGE(R"((____|     `._____  )" << copyright);
						MESSAGE(R"( ____|       _|___)" "");
						MESSAGE(R"((____|     .'       This program may be freely redistributed under)" "");
						MESSAGE(R"(     |____/         the terms of the MIT License.)" "");
					}

					else if (args[1] == "load") {
						packageManager->Reload();
						if (!options.contains("--ignore") && !options.contains("-i")) {
							if (packageManager->HasMissedPackages()) {
								ERROR("Plugin manager has missing packages, run 'update --missing' to resolve issues.");
								continue;
							}
							if (packageManager->HasConflictedPackages()) {
								ERROR("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
								continue;
							}
						}
						if (pluginManager->IsInitialized()) {
							ERROR("Plugin manager already loaded.");
						} else {
							pluginManager->Initialize();
							MESSAGE("Plugin manager was loaded.");
						}
					}

					else if (args[1] == "unload") {
						if (!pluginManager->IsInitialized()) {
							ERROR("Plugin manager already unloaded.");
						} else {
							pluginManager->Terminate();
							MESSAGE("Plugin manager was unloaded.");
						}
						packageManager->Reload();
					}

					else if (args[1] == "reload") {
						if (!pluginManager->IsInitialized()) {
							ERROR("Plugin manager not loaded.");
							packageManager->Reload();
						} else {
							pluginManager->Terminate();
							packageManager->Reload();
							pluginManager->Initialize();
							MESSAGE("Plugin manager was reloaded.");
						}
					}

					else if (args[1] == "plugins") {
						if (!pluginManager->IsInitialized()) {
							ERROR("You must load plugin manager before query any information from it.");
							continue;
						}
						auto plugins = pluginManager->GetPlugins();
						auto count = plugins.size();
						if (!count) {
							ERROR("No plugins loaded.");
						} else {
							MESSAGE_FMT("Listing {} plugin{}:", count, (count > 1) ? "s" : "");
						}
						for (const auto& plugin : plugins) {
							Print<PluginState>(plugin, PluginUtils::ToString);
						}
					}

					else if (args[1] == "modules") {
						if (!pluginManager->IsInitialized()) {
							ERROR("You must load plugin manager before query any information from it.");
							continue;
						}
						auto modules = pluginManager->GetModules();
						auto count = modules.size();
						if (!count) {
							ERROR("No modules loaded.");
						} else {
							MESSAGE_FMT("Listing {} module{}:", count, (count > 1) ? "s" : "");
						}
						for (const auto& module : modules) {
							Print<ModuleState>(module, ModuleUtils::ToString);
						}
					}

					else if (args[1] == "plugin") {
						if (args.size() > 2) {
							if (!pluginManager->IsInitialized()) {
								ERROR("You must load plugin manager before query any information from it.");
								continue;
							}
							auto plugin = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindPluginFromId(FormatInt(args[2])) : pluginManager->FindPlugin(args[2]);
							if (plugin) {
								Print<PluginState>("Plugin", plugin, PluginUtils::ToString);
								auto descriptor = plugin.GetDescriptor();
								MESSAGE_FMT("  Language module: {}", descriptor.GetLanguageModule());
								MESSAGE("  Dependencies: ");
								for (const auto& reference : descriptor.GetDependencies()) {
									auto dependency = pluginManager->FindPlugin(reference.GetName());
									if (dependency) {
										Print<PluginState>(dependency, PluginUtils::ToString, "    ");
									} else {
										auto version = reference.GetRequestedVersion();
										MESSAGE_FMT("    {} <Missing> (v{})", reference.GetName(), version.has_value() ? version->to_string() : "[latest]");
									}
								}
								MESSAGE_FMT("  File: {}", descriptor.GetEntryPoint());
							} else {
								ERROR_FMT("Plugin {} not found.", args[2]);
							}
						} else {
							ERROR("You must provide name.");
						}
					}

					else if (args[1] == "module") {
						if (args.size() > 2) {
							if (!pluginManager->IsInitialized()) {
								ERROR("You must load plugin manager before query any information from it.");
								continue;
							}
							auto module = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindModuleFromId(FormatInt(args[2])) : pluginManager->FindModule(args[2]);
							if (module) {
								Print<ModuleState>("Module", module, ModuleUtils::ToString);
								MESSAGE_FMT("  Language: {}", module.GetLanguage());
								MESSAGE_FMT("  File: {}", std::filesystem::path(module.GetFilePath()).string());
							} else {
								ERROR_FMT("Module {} not found.", args[2]);
							}
						} else {
							ERROR("You must provide name.");
						}
					}

					else if (args[1] == "snapshot") {
						if (pluginManager->IsInitialized()) {
							ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						packageManager->SnapshotPackages(plug->GetConfig().baseDir / std::format("snapshot_{}.wpackagemanifest", DateTime::Get("%Y_%m_%d_%H_%M_%S")), true);
					}

					else if (args[1] == "repo") {
						if (pluginManager->IsInitialized()) {
							ERROR("You must unload plugin manager before bring any change with package manager.");
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
							ERROR("You must give at least one repository to add.");
						}
					}

					else if (args[1] == "install") {
						if (pluginManager->IsInitialized()) {
							ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (options.contains("--missing") || options.contains("-m")) {
							if (packageManager->HasMissedPackages()) {
								packageManager->InstallMissedPackages();
							} else {
								MESSAGE("No missing packages were found.");
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
								ERROR("You must give at least one requirement to install.");
							}
						}
					}

					else if (args[1] == "remove") {
						if (pluginManager->IsInitialized()) {
							ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (options.contains("--all") || options.contains("-a")) {
							packageManager->UninstallAllPackages();
						} else if (options.contains("--conflict") || options.contains("-c")) {
							if (packageManager->HasConflictedPackages()) {
								packageManager->UninstallConflictedPackages();
							} else {
								MESSAGE("No conflicted packages were found.");
							}
						} else {
							if (args.size() > 2) {
								packageManager->UninstallPackages(std::span(args.begin() + 2, args.size() - 2));
							} else {
								ERROR("You must give at least one requirement to remove.");
							}
						}
					}

					else if (args[1] == "update") {
						if (pluginManager->IsInitialized()) {
							ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (options.contains("--all") || options.contains("-a")) {
							packageManager->UpdateAllPackages();
						} else {
							if (args.size() > 2) {
								packageManager->UpdatePackages(std::span(args.begin() + 2, args.size() - 2));
							} else {
								ERROR("You must give at least one requirement to update.");
							}
						}
					}

					else if (args[1] == "list") {
						if (pluginManager->IsInitialized()) {
							ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						auto localPackages = packageManager->GetLocalPackages();
						auto count = localPackages.size();
						if (!count) {
							ERROR("No local packages found.");
						} else {
							MESSAGE_FMT("Listing {} local package{}:", count, (count > 1) ? "s" : "");
						}
						for (const auto& localPackage : localPackages) {
							MESSAGE_FMT("  {} [{}] (v{}) at {}", localPackage->name, localPackage->type, localPackage->version, localPackage->path.string());
						}
					}

					else if (args[1] == "query") {
						if (pluginManager->IsInitialized()) {
							ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						auto remotePackages = packageManager->GetRemotePackages();
						auto count = remotePackages.size();
						if (!count) {
							ERROR("No remote packages found.");
						} else {
							MESSAGE_FMT("Listing {} remote package{}:", count, (count > 1) ? "s" : "");
						}
						for (const auto& remotePackage : remotePackages) {
							if (remotePackage->author.empty() || remotePackage->description.empty()) {
								MESSAGE_FMT("  {} [{}]", remotePackage->name, remotePackage->type);
							} else {
								MESSAGE_FMT("  {} [{}] ({}) by {}", remotePackage->name, remotePackage->type, remotePackage->description, remotePackage->author);
							}
						}
					}

					else if (args[1] == "show") {
						if (pluginManager->IsInitialized()) {
							ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (args.size() > 2) {
							auto package = packageManager->FindLocalPackage(args[2]);
							if (package) {
								MESSAGE_FMT("  Name: {}", package->name);
								MESSAGE_FMT("  Type: {}", package->type);
								MESSAGE_FMT("  Version: {}", package->version);
								MESSAGE_FMT("  File: {}", package->path.string());

							} else {
								ERROR_FMT("Package {} not found.", args[2]);
							}
						} else {
							ERROR("You must provide name.");
						}
					}

					else if (args[1] == "search") {
						if (pluginManager->IsInitialized()) {
							ERROR("You must unload plugin manager before bring any change with package manager.");
							continue;
						}
						if (args.size() > 2) {
							auto package = packageManager->FindRemotePackage(args[2]);
							if (package) {
								MESSAGE_FMT("  Name: {}", package->name);
								MESSAGE_FMT("  Type: {}", package->type);
								if (!package->author.empty()) {
									MESSAGE_FMT("  Author: {}", package->author);
								}
								if (!package->description.empty()) {
									MESSAGE_FMT("  Description: {}", package->description);
								}
								const auto& versions = package->versions;
								if (!versions.empty()) {
									std::string combined("  Versions: ");
									std::format_to(std::back_inserter(combined), "{}", versions.begin()->version);
									for (auto it = std::next(versions.begin()); it != versions.end(); ++it) {
										std::format_to(std::back_inserter(combined), ", {}", it->version);
									}
									MESSAGE(combined);
								} else {
									MESSAGE("");
								}
							} else {
								ERROR_FMT("Package {} not found.", args[2]);
							}
						} else {
							ERROR("You must provide name.");
						}
					}

					else {
						ERROR_FMT("unknown option: {}", args[1]);
						ERROR("usage: plg <command> [options] [arguments]");
						ERROR("Try plg help or -h for more information.");
					}
				}

				plug->Update();
			} else {
				ERROR("usage: plg <command> [options] [arguments]");
				ERROR("Try plg help or -h for more information.");
			}
        }
    }

    return EXIT_SUCCESS;
}
