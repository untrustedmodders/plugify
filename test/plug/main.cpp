#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

#include <CLI/CLI.hpp>
#include <glaze/glaze.hpp>
#include <plg/format.hpp>

#include "plugify/extension.hpp"
#include "plugify/logger.hpp"
#include "plugify/manager.hpp"
#include "plugify/plugify.hpp"

#if PLUGIFY_PLATFORM_APPLE
#include <unistd.h>
#endif

namespace plg {
	template <string_like First>
	void print(First&& first) {
		std::cout << first << std::endl;
	}

	template <typename... Args>
	void print(std::format_string<Args...> fmt, Args&&... args) {
		std::cout << std::format(fmt, std::forward<Args>(args)...) << std::endl;
	}
}

using namespace plugify;

namespace {
	constexpr std::string_view SEPARATOR_LINE = "--------------------------------------------------------------------------------";
	constexpr std::string_view DOUBLE_LINE = "================================================================================";

	// ANSI Color codes
	struct Colors {
		static constexpr std::string_view RESET = "\033[0m";
		static constexpr std::string_view BOLD = "\033[1m";
		static constexpr std::string_view RED = "\033[31m";
		static constexpr std::string_view GREEN = "\033[32m";
		static constexpr std::string_view YELLOW = "\033[33m";
		static constexpr std::string_view BLUE = "\033[34m";
		static constexpr std::string_view MAGENTA = "\033[35m";
		static constexpr std::string_view CYAN = "\033[36m";
		static constexpr std::string_view GRAY = "\033[90m";
		static constexpr std::string_view BRIGHT_RED = "\033[91m";
		static constexpr std::string_view BRIGHT_GREEN = "\033[92m";
		static constexpr std::string_view BRIGHT_YELLOW = "\033[93m";
	};

	// Global flag for color output (can be set via command line)
	bool g_useColor = true;

	// Helper to apply color
	std::string Colorize(std::string_view text, std::string_view color) {
		if (!g_useColor) {
			return std::string(text);
		}
		return std::format("{}{}{}", color, text, Colors::RESET);
	}

	// Helper to format duration
	std::string FormatDuration(std::chrono::milliseconds duration) {
		using namespace std::chrono;

		auto ns = duration_cast<nanoseconds>(duration).count();
		if (ns < 1000) {
			return std::format("{}ns", ns);
		} else if (ns < 1000000) {
			return std::format("{:.2f}μs", static_cast<double>(ns) / 1000.0);
		} else if (ns < 1000000000) {
			return std::format("{:.2f}ms", static_cast<double>(ns) / 1000000.0);
		} else {
			return std::format("{:.2f}s", static_cast<double>(ns) / 1000000000.0);
		}
	}

	struct Glyphs {
		std::string_view Ok;
		std::string_view Fail;
		std::string_view Warning;
		std::string_view Skipped;
		std::string_view Valid;
		std::string_view Resolving;
		std::string_view Arrow;
		std::string_view Number;
		std::string_view Unknown;
		std::string_view Missing;
		std::string_view Equal;
		std::string_view NotEqual;
		std::string_view Running;
	};

	// Helper to get state color/symbol
	struct StateInfo {
		std::string_view symbol;
		std::string_view color;
	};

	// Unicode (original)
	inline constexpr Glyphs UnicodeGlyphs{ "✓", "✗", "⚠", "○", "●", "⋯", "→",
		                                   "#", "?", "ℹ", "=", "≠", "⚙" };

	// Plain ASCII fallback
	inline constexpr Glyphs AsciiGlyphs{ "v", "x", "!", "o", "*",  "...", "->",
		                                 "#", "?", "i", "=", "!=", ">>" };

	// selection strategy examples:

#define USE_ASCII
	// 1) Compile-time: define USE_ASCII or USE_ASCII_ANSI in your build
#if defined(USE_ASCII)
	inline constexpr const Glyphs& Icons = AsciiGlyphs;
#else
	inline constexpr const Glyphs& Icons = UnicodeGlyphs;
#endif

	StateInfo GetStateInfo(ExtensionState state) {
		switch (state) {
			case ExtensionState::Parsed:
			case ExtensionState::Resolved:
			case ExtensionState::Started:
			case ExtensionState::Loaded:
			case ExtensionState::Exported:
				return { Icons.Ok, Colors::MAGENTA };
			case ExtensionState::Failed:
			case ExtensionState::Corrupted:
				return { Icons.Fail, Colors::RED };
			case ExtensionState::Unresolved:
				return { Icons.Warning, Colors::YELLOW };
			case ExtensionState::Disabled:
			case ExtensionState::Skipped:
				return { Icons.Skipped, Colors::GRAY };
			case ExtensionState::Loading:
			case ExtensionState::Starting:
			case ExtensionState::Parsing:
			case ExtensionState::Resolving:
			case ExtensionState::Exporting:
			case ExtensionState::Ending:
			case ExtensionState::Terminating:
				return { Icons.Resolving, Colors::CYAN };
			case ExtensionState::Running:
				return { Icons.Running, Colors::GREEN };
			default:
				return { Icons.Unknown, Colors::GRAY };
		}
	}

	// Helper to truncate string with ellipsis
	std::string Truncate(const std::string& str, size_t maxLen) {
		if (str.length() <= maxLen) {
			return str;
		}
		return str.substr(0, maxLen - 3) + "...";
	}

	// Helper to format file size
	std::uintmax_t GetSizeRecursive(const std::filesystem::path& path) {
		namespace fs = std::filesystem;
		std::uintmax_t totalSize = 0;

		std::error_code ec;
		if (fs::is_regular_file(path, ec)) {
			auto sz = fs::file_size(path, ec);
			if (!ec) {
				totalSize += sz;
			}
		} else if (fs::is_directory(path)) {
			for (auto& entry : fs::recursive_directory_iterator(
			         path,
			         fs::directory_options::skip_permission_denied,
			         ec
			     )) {
				if (fs::is_regular_file(entry, ec)) {
					auto sz = fs::file_size(entry, ec);
					if (!ec) {
						totalSize += sz;
					}
				}
			}
		}
		return totalSize;
	}

	std::string FormatFileSize(const std::filesystem::path& path) {
		try {
			auto size = GetSizeRecursive(path);
			if (size < 1024) {
				return std::format("{} B", size);
			} else if (size < 1024 * 1024) {
				return std::format("{:.1f} KB", static_cast<double>(size) / 1024.0);
			} else if (size < 1024ull * 1024 * 1024) {
				return std::format("{:.1f} MB", static_cast<double>(size) / (1024.0 * 1024.0));
			} else {
				return std::format("{:.1f} GB", static_cast<double>(size) / (1024.0 * 1024.0 * 1024.0));
			}
		} catch (...) {
			return "N/A";
		}
	}

	// Filter options
	struct FilterOptions {
		std::optional<std::vector<ExtensionState>> states;
		std::optional<std::vector<std::string>> languages;
		bool showOnlyFailed = false;
		bool showOnlyWithErrors = false;
		std::optional<std::string> searchQuery;
	};

	// Sort options
	enum class SortBy { Name, Version, State, Language, LoadTime };

	// Helper function to parse comma-separated values
	std::vector<std::string> ParseCsv(const std::string& str) {
		std::vector<std::string> result;
		std::stringstream ss(str);
		std::string item;
		while (std::getline(ss, item, ',')) {
			// Trim whitespace
			item.erase(0, item.find_first_not_of(" \t"));
			item.erase(item.find_last_not_of(" \t") + 1);
			if (!item.empty()) {
				result.push_back(item);
			}
		}
		return result;
	};

	// Helper to parse sort option
	SortBy ParseSortBy(std::string_view str) {
		if (str == "name") {
			return SortBy::Name;
		}
		if (str == "version") {
			return SortBy::Version;
		}
		if (str == "state") {
			return SortBy::State;
		}
		if (str == "language") {
			return SortBy::Language;
		}
		if (str == "loadtime") {
			return SortBy::LoadTime;
		}
		return SortBy::Name;
	};

	// Helper to parse state filter
	std::vector<ExtensionState> ParseStates(const std::vector<std::string>& strs) {
		std::vector<ExtensionState> states;
		states.reserve(strs.size());
		for (const auto& str : strs) {
			std::string lower = str;
			std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

			if (lower == "loaded") {
				states.push_back(ExtensionState::Loaded);
			} else if (lower == "started") {
				states.push_back(ExtensionState::Started);
			} else if (lower == "failed") {
				states.push_back(ExtensionState::Failed);
			} else if (lower == "disabled") {
				states.push_back(ExtensionState::Disabled);
			} else if (lower == "corrupted") {
				states.push_back(ExtensionState::Corrupted);
			} else if (lower == "unresolved") {
				states.push_back(ExtensionState::Unresolved);
			}
			// Add more as needed
		}
		return states;
	};

	UniqueId FormatId(std::string_view str) {
		UniqueId::Value result;
		auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);

		if (ec != std::errc{}) {
			plg::print("Error: {}", std::make_error_code(ec).message());
			return {};
		} else if (ptr != str.data() + str.size()) {
			plg::print("Invalid argument: trailing characters after the valid part");
			return {};
		}

		return UniqueId{ result };
	}

	// Helper to check if extension matches filter
	bool MatchesFilter(const Extension* ext, const FilterOptions& filter) {
		if (filter.showOnlyFailed && ext->GetState() != ExtensionState::Failed) {
			return false;
		}

		if (filter.showOnlyWithErrors && !ext->HasErrors()) {
			return false;
		}

		if (filter.states.has_value()) {
			auto state = ext->GetState();
			if (std::find(filter.states->begin(), filter.states->end(), state)
			    == filter.states->end()) {
				return false;
			}
		}

		if (filter.languages.has_value()) {
			auto& lang = ext->GetLanguage();
			if (std::find(filter.languages->begin(), filter.languages->end(), lang)
			    == filter.languages->end()) {
				return false;
			}
		}

		if (filter.searchQuery.has_value()) {
			std::string query = filter.searchQuery.value();
			std::transform(query.begin(), query.end(), query.begin(), ::tolower);

			std::string name = ext->GetName();
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);

			std::string desc = ext->GetDescription();
			std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

			if (name.find(query) == std::string::npos && desc.find(query) == std::string::npos) {
				return false;
			}
		}

		return true;
	}

	// Convert extension to JSON
	glz::json_t ExtensionToJson(const Extension* ext) {
		glz::json_t j;
		j["id"] = UniqueId::Value{ ext->GetId() };
		j["name"] = ext->GetName();
		j["version"] = ext->GetVersionString();
		j["type"] = ext->IsPlugin() ? "plugin" : "module";
		j["state"] = plg::enum_to_string(ext->GetState());
		j["language"] = ext->GetLanguage();
		j["location"] = plg::as_string(ext->GetLocation());

		if (!ext->GetDescription().empty()) {
			j["description"] = ext->GetDescription();
		}
		if (!ext->GetAuthor().empty()) {
			j["author"] = ext->GetAuthor();
		}
		if (!ext->GetWebsite().empty()) {
			j["website"] = ext->GetWebsite();
		}
		if (!ext->GetLicense().empty()) {
			j["license"] = ext->GetLicense();
		}

		// Dependencies
		if (!ext->GetDependencies().empty()) {
			glz::json_t::array_t dependencies;
			for (const auto& dep : ext->GetDependencies()) {
				glz::json_t::object_t depJson;
				depJson["name"] = dep.GetName();
				depJson["constraints"] = dep.GetConstraints().to_string();
				depJson["optional"] = dep.IsOptional();
				dependencies.emplace_back(std::move(depJson));
			}
			j["dependencies"] = glz::json_t::array_t{ std::move(dependencies) };
		}

		// Performance
		j["performance"]["total_time_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(
		                                        ext->GetTotalTime()
		)
		                                        .count();

		// Errors and warnings
		if (ext->HasErrors()) {
			glz::json_t::array_t errors;
			errors.reserve(ext->GetErrors().size());
			for (const auto& error : ext->GetErrors()) {
				errors.emplace_back(error);
			}
			j["errors"] = glz::json_t::array_t{ std::move(errors) };
		}
		if (ext->HasWarnings()) {
			glz::json_t::array_t warnings;
			warnings.reserve(ext->GetWarnings().size());
			for (const auto& warning : ext->GetWarnings()) {
				warnings.emplace_back(warning);
			}
			j["warnings"] = glz::json_t::array_t{ std::move(warnings) };
		}

		return j;
	}

	// Filter extensions based on criteria
	std::vector<const Extension*>
	FilterExtensions(const std::vector<const Extension*>& extensions, const FilterOptions& filter) {
		std::vector<const Extension*> result;

		for (const auto& ext : extensions) {
			if (!MatchesFilter(ext, filter)) {
				continue;
			}
			result.push_back(ext);
		}

		return result;
	}

	// Sort extensions
	void SortExtensions(std::vector<const Extension*>& extensions, SortBy sortBy, bool reverse = false) {
		std::sort(
		    extensions.begin(),
		    extensions.end(),
		    [sortBy, reverse](const Extension* a, const Extension* b) {
			    bool result = false;
			    switch (sortBy) {
				    case SortBy::Name:
					    result = a->GetName() < b->GetName();
					    break;
				    case SortBy::Version:
					    result = a->GetVersion() < b->GetVersion();
					    break;
				    case SortBy::State:
					    result = a->GetState() < b->GetState();
					    break;
				    case SortBy::Language:
					    result = a->GetLanguage() < b->GetLanguage();
					    break;
				    case SortBy::LoadTime:
					    result = a->GetOperationTime(ExtensionState::Loaded)
					             < b->GetOperationTime(ExtensionState::Loaded);
					    break;
			    }
			    return reverse ? !result : result;
		    }
		);
	}

	// Dependency tree builder
	void PrintDependencyTree(
	    const Extension* ext,
	    const Manager& manager,
	    const std::string& prefix = "",
	    bool isLast = true
	) {
		// Print current extension
		std::string connector = isLast ? "└─ " : "├─ ";
		auto [symbol, color] = GetStateInfo(ext->GetState());

		plg::print(
		    "{}{}{} {} {} {}",
		    prefix,
		    connector,
		    Colorize(symbol, color),
		    Colorize(ext->GetName(), Colors::BOLD),
		    Colorize(ext->GetVersionString(), Colors::GRAY),
		    ext->HasErrors() ? Colorize("[ERROR]", Colors::RED) : ""
		);

		// Print dependencies
		const auto& deps = ext->GetDependencies();
		std::string newPrefix = prefix + (isLast ? "    " : "│   ");

		for (size_t i = 0; i < deps.size(); ++i) {
			const auto& dep = deps[i];
			bool lastDep = (i == deps.size() - 1);

			// Try to find the actual dependency
			if (auto depExt = manager.FindExtension(dep.GetName())) {
				PrintDependencyTree(depExt, manager, newPrefix, lastDep);
			} else {
				std::string depConnector = lastDep ? "└─ " : "├─ ";
				std::string status = dep.IsOptional() ? "[optional]" : "[required]";
				plg::print(
				    "{}{}{} {} {} {} {}",
				    newPrefix,
				    depConnector,
				    Icons.Skipped,
				    dep.GetName(),
				    dep.GetConstraints().to_string(),
				    Colorize(status, Colors::GRAY),
				    Colorize("[NOT FOUND]", Colors::YELLOW)
				);
			}
		}
	}

	// Calculate health score
	struct HealthReport {
		size_t score = 100;  // 0-100
		std::vector<std::string> issues;
		std::vector<std::string> warnings;
		std::map<std::string, size_t> statistics;
	};

	HealthReport CalculateSystemHealth(const Manager& manager) {
		HealthReport report;

		auto allExtensions = manager.GetExtensions();
		report.statistics["total_extensions"] = allExtensions.size();

		size_t failedCount = 0;
		size_t errorCount = 0;
		size_t warningCount = 0;
		size_t slowLoadCount = 0;

		for (const auto& ext : allExtensions) {
			if (ext->GetState() == ExtensionState::Failed
			    || ext->GetState() == ExtensionState::Corrupted) {
				++failedCount;
				report.issues.push_back(std::format("{} is in failed state", ext->GetName()));
			}

			errorCount += ext->GetErrors().size();
			warningCount += ext->GetWarnings().size();

			// Check for slow loading (> 1 second)
			auto loadTime = ext->GetOperationTime(ExtensionState::Loaded);
			if (std::chrono::duration_cast<std::chrono::milliseconds>(loadTime).count() > 1000) {
				++slowLoadCount;
				report.warnings.push_back(
				    std::format("{} took {} to load", ext->GetName(), FormatDuration(loadTime))
				);
			}
		}

		report.statistics["failed_extensions"] = failedCount;
		report.statistics["extensions_with_errors"] = errorCount;
		report.statistics["total_warnings"] = warningCount;
		report.statistics["slow_loading_extensions"] = slowLoadCount;

		// Calculate score
		if (failedCount > 0) {
			report.score -= failedCount * 15;
		}
		if (errorCount > 0) {
			report.score -= errorCount * 10;
		}
		if (warningCount > 0) {
			report.score -= std::min<size_t>(warningCount * 2, 20);
		}
		if (slowLoadCount > 0) {
			report.score -= std::min<size_t>(slowLoadCount * 5, 15);
		}

		report.score = std::max<size_t>(0, report.score);

		return report;
	}
}

// Enhanced PlugifyApp methods:

class PlugifyApp {
public:
	explicit PlugifyApp(std::shared_ptr<Plugify> plugify)
	    : plug(std::move(plugify)) {
	}

	void Initialize() {
		if (!plug->Initialize()) {
			plg::print("{}: No feet, no sweets!.", Colorize("Error", Colors::RED));
			throw std::runtime_error("Failed to initialize Plugify");
		}
		const auto& manager = plug->GetManager();
		if (auto initResult = manager.Initialize()) {
			plg::print("Plugin system initialized.");
		} else {
			plg::print("{}: {}.", Colorize("Error", Colors::RED), initResult.error());
			throw std::runtime_error("Failed to initialize Manager");
		}
	}

	void Terminate() {
		plug->Terminate();
		plg::print("Plugin system terminated.");
	}

	std::string GetVersionString() {
		constexpr auto year = std::string_view(__DATE__).substr(7, 4);
		return std::format(
		    ""
		    R"(      ____)"
		    "\n"
		    R"( ____|    \         Plugify {})"
		    "\n"
		    R"((____|     `._____  Copyright (C) 2023-{} Untrusted Modders Team)"
		    "\n"
		    R"( ____|       _|___)"
		    "\n"
		    R"((____|     .'       This program may be freely redistributed under)"
		    "\n"
		    R"(     |____/         the terms of the MIT License.)",
		    plug->GetVersion(),
		    year
		);
	}

	void LoadManager() {
		if (!plug->IsInitialized()) {
			plg::print("{}: Initialize system before use.", Colorize("Error", Colors::RED));
			return;
		}
		const auto& manager = plug->GetManager();
		if (manager.IsInitialized()) {
			plg::print("{}: Plugin manager already loaded.", Colorize("Error", Colors::RED));
		} else {
			if (auto initResult = manager.Initialize()) {
				plg::print("Plugin manager was loaded.");
			} else {
				plg::print("{}: {}.", Colorize("Error", Colors::RED), initResult.error());
			}
		}
	}

	void UnloadManager() {
		if (!plug->IsInitialized()) {
			plg::print("{}: Initialize system before use.", Colorize("Error", Colors::RED));
			return;
		}
		const auto& manager = plug->GetManager();
		if (!manager.IsInitialized()) {
			plg::print("{}: Plugin manager already unloaded.", Colorize("Error", Colors::RED));
		} else {
			manager.Terminate();
			plg::print("Plugin manager was unloaded.");
		}
	}

	void ReloadManager() {
		if (!plug->IsInitialized()) {
			plg::print("{}: Initialize system before use.", Colorize("Error", Colors::RED));
			return;
		}
		const auto& manager = plug->GetManager();
		if (!manager.IsInitialized()) {
			plg::print("Plugin manager not loaded.");
		} else {
			manager.Terminate();
			if (auto initResult = manager.Initialize()) {
				plg::print("Plugin manager was reloaded.");
			} else {
				plg::print("{}: {}.", Colorize("Error", Colors::RED), initResult.error());
			}
		}
	}

	const Manager& GetManager() const {
		return plug->GetManager();
	}

	void ListPlugins(
	    const FilterOptions& filter = {},
	    SortBy sortBy = SortBy::Name,
	    bool reverseSort = false,
	    bool jsonOutput = false
	) {
		if (!CheckManager()) {
			return;
		}

		const auto& manager = plug->GetManager();
		auto plugins = manager.GetExtensionsByType(ExtensionType::Plugin);

		// Apply filters
		auto filtered = FilterExtensions(plugins, filter);
		SortExtensions(filtered, sortBy, reverseSort);

		// Output
		if (jsonOutput) {
			glz::json_t::array_t objects;
			objects.reserve(filtered.size());
			for (const auto& plugin : filtered) {
				objects.emplace_back(ExtensionToJson(plugin));
			}
			plg::print(*glz::json_t{ std::move(objects) }.dump());
			return;
		}

		auto count = filtered.size();
		if (!count) {
			plg::print(Colorize("No plugins found matching criteria.", Colors::YELLOW));
			return;
		}

		plg::print(
		    "{}:",
		    Colorize(std::format("Listing {} plugin{}", count, (count > 1) ? "s" : ""), Colors::BOLD)
		);
		plg::print(SEPARATOR_LINE);

		// Header
		plg::print(
		    "{} {} {} {} {} {}",
		    Colorize(std::format("{:<3}", Icons.Number), Colors::GRAY),
		    Colorize(std::format("{:<25}", "Name"), Colors::GRAY),
		    Colorize(std::format("{:<15}", "Version"), Colors::GRAY),
		    Colorize(std::format("{:<12}", "State"), Colors::GRAY),
		    Colorize(std::format("{:<8}", "Lang"), Colors::GRAY),
		    Colorize(std::format("{:<12}", "Load Time"), Colors::GRAY)
		);
		plg::print(SEPARATOR_LINE);

		size_t index = 1;
		for (const auto& plugin : filtered) {
			auto state = plugin->GetState();
			auto stateStr = plg::enum_to_string(state);
			auto [symbol, color] = GetStateInfo(state);
			const auto& name = !plugin->GetName().empty()
			                       ? plugin->GetName()
			                       : plg::as_string(plugin->GetLocation().filename());

			// Get load time if available
			std::string loadTime = "N/A";
			try {
				auto duration = plugin->GetOperationTime(ExtensionState::Loaded);
				if (duration.count() > 0) {
					loadTime = FormatDuration(duration);
				}
			} catch (...) {
			}

			plg::print(
			    "{:<3} {:<25} {:<15} {} {:<11} {:<8} {:<12}",
			    index++,
			    Truncate(name, 24),
			    plugin->GetVersionString(),
			    Colorize(symbol, color),
			    Truncate(std::string(stateStr), 10),
			    Truncate(plugin->GetLanguage(), 7),
			    loadTime
			);

			// Show errors/warnings if any
			if (plugin->HasErrors()) {
				for (const auto& error : plugin->GetErrors()) {
					plg::print("     └─ {}: {}", Colorize("Error", Colors::RED), error);
				}
			}
			if (plugin->HasWarnings()) {
				for (const auto& warning : plugin->GetWarnings()) {
					plg::print("     └─ {}: {}", Colorize("Warning", Colors::YELLOW), warning);
				}
			}
		}
		plg::print(SEPARATOR_LINE);

		// Summary
		if (filter.states.has_value() || filter.languages.has_value()
		    || filter.searchQuery.has_value() || filter.showOnlyFailed) {
			plg::print(
			    "{}",
			    Colorize(
			        std::format("Filtered: {} of {} total plugins shown", filtered.size(), plugins.size()),
			        Colors::GRAY
			    )
			);
		}
	}

	void ListModules(
	    const FilterOptions& filter = {},
	    SortBy sortBy = SortBy::Name,
	    bool reverseSort = false,
	    bool jsonOutput = false
	) {
		if (!CheckManager()) {
			return;
		}

		const auto& manager = plug->GetManager();
		auto modules = manager.GetExtensionsByType(ExtensionType::Module);

		// Apply filters
		auto filtered = FilterExtensions(modules, filter);
		SortExtensions(filtered, sortBy, reverseSort);

		// Output
		if (jsonOutput) {
			glz::json_t::array_t objects;
			objects.reserve(filtered.size());
			for (const auto& module : filtered) {
				objects.emplace_back(ExtensionToJson(module));
			}
			plg::print(*glz::json_t{ std::move(objects) }.dump());
			return;
		}

		auto count = filtered.size();
		if (!count) {
			plg::print(Colorize("No modules found matching criteria.", Colors::YELLOW));
			return;
		}

		plg::print(
		    "{}:",
		    Colorize(std::format("Listing {} module{}", count, (count > 1) ? "s" : ""), Colors::BOLD)
		);
		plg::print(SEPARATOR_LINE);

		// Header
		plg::print(
		    "{} {} {} {} {} {}",
		    Colorize(std::format("{:<3}", Icons.Number), Colors::GRAY),
		    Colorize(std::format("{:<25}", "Name"), Colors::GRAY),
		    Colorize(std::format("{:<15}", "Version"), Colors::GRAY),
		    Colorize(std::format("{:<12}", "State"), Colors::GRAY),
		    Colorize(std::format("{:<8}", "Lang"), Colors::GRAY),
		    Colorize(std::format("{:<12}", "Load Time"), Colors::GRAY)
		);
		plg::print(SEPARATOR_LINE);

		size_t index = 1;
		for (const auto& module : filtered) {
			auto state = module->GetState();
			auto stateStr = plg::enum_to_string(state);
			auto [symbol, color] = GetStateInfo(state);

			// Get load time if available
			std::string loadTime = "N/A";
			try {
				auto duration = module->GetOperationTime(ExtensionState::Loaded);
				if (duration.count() > 0) {
					loadTime = FormatDuration(duration);
				}
			} catch (...) {
			}

			plg::print(
			    "{:<3} {:<25} {:<15} {} {:<11} {:<8} {:<12}",
			    index++,
			    Truncate(module->GetName(), 24),
			    module->GetVersionString(),
			    Colorize(symbol, color),
			    Truncate(std::string(stateStr), 10),
			    Truncate(module->GetLanguage(), 7),
			    loadTime
			);

			// Show errors/warnings if any
			if (module->HasErrors()) {
				for (const auto& error : module->GetErrors()) {
					plg::print("     └─ {}: {}", Colorize("Error", Colors::RED), error);
				}
			}
			if (module->HasWarnings()) {
				for (const auto& warning : module->GetWarnings()) {
					plg::print("     └─ {}: {}", Colorize("Warning", Colors::YELLOW), warning);
				}
			}
		}
		plg::print(SEPARATOR_LINE);

		// Summary
		if (filter.states.has_value() || filter.languages.has_value()
		    || filter.searchQuery.has_value() || filter.showOnlyFailed) {
			plg::print(
			    "{}",
			    Colorize(
			        std::format("Filtered: {} of {} total modules shown", filtered.size(), modules.size()),
			        Colors::GRAY
			    )
			);
		}
	}

	void ShowPlugin(std::string_view identifier, bool plugin_use_id, bool jsonOutput) {
		if (!CheckManager()) {
			return;
		}

		const auto& manager = plug->GetManager();
		auto plugin = plugin_use_id ? manager.FindExtension(FormatId(identifier))
		                            : manager.FindExtension(identifier);

		if (!plugin) {
			if (jsonOutput) {
				glz::json_t error;
				error["error"] = std::format("Plugin {} not found", identifier);
				plg::print(*error.dump());
			} else {
				plg::print("{}: Plugin {} not found.", Colorize("Error", Colors::RED), identifier);
			}
			return;
		}

		if (!plugin->IsPlugin()) {
			if (jsonOutput) {
				glz::json_t error;
				error["error"] = std::format("'{}' is not a plugin (it's a module)", identifier);
				plg::print(*error.dump());
			} else {
				plg::print(
				    "{}: '{}' is not a plugin (it's a module).",
				    Colorize("Error", Colors::RED),
				    identifier
				);
			}
			return;
		}

		// JSON output
		if (jsonOutput) {
			plg::print(*ExtensionToJson(plugin).dump());
			return;
		}

		// Display detailed plugin information with colors
		plg::print(DOUBLE_LINE);
		plg::print(
		    "{}: {}",
		    Colorize("PLUGIN INFORMATION", Colors::BOLD),
		    Colorize(plugin->GetName(), Colors::CYAN)
		);
		plg::print(DOUBLE_LINE);

		// Status indicator
		auto [symbol, stateColor] = GetStateInfo(plugin->GetState());
		plg::print(
		    "\n{} {} {}",
		    Colorize(symbol, stateColor),
		    Colorize("Status:", Colors::BOLD),
		    Colorize(plg::enum_to_string(plugin->GetState()), stateColor)
		);

		// Basic Information
		plg::print(Colorize("\n[Basic Information]", Colors::CYAN));
		plg::print("  {:<15} {}", Colorize("ID:", Colors::GRAY), plugin->GetId());
		plg::print("  {:<15} {}", Colorize("Name:", Colors::GRAY), plugin->GetName());
		plg::print(
		    "  {:<15} {}",
		    Colorize("Version:", Colors::GRAY),
		    Colorize(plugin->GetVersionString(), Colors::GREEN)
		);
		plg::print("  {:<15} {}", Colorize("Language:", Colors::GRAY), plugin->GetLanguage());
		plg::print(
		    "  {:<15} {}",
		    Colorize("Location:", Colors::GRAY),
		    plg::as_string(plugin->GetLocation())
		);
		plg::print(
		    "  {:<15} {}",
		    Colorize("File Size:", Colors::GRAY),
		    FormatFileSize(plugin->GetLocation())
		);

		// Optional Information
		if (!plugin->GetDescription().empty() || !plugin->GetAuthor().empty()
		    || !plugin->GetWebsite().empty() || !plugin->GetLicense().empty()) {
			plg::print(Colorize("\n[Additional Information]", Colors::CYAN));
			if (!plugin->GetDescription().empty()) {
				plg::print(
				    "  {:<15} {}",
				    Colorize("Description:", Colors::GRAY),
				    plugin->GetDescription()
				);
			}
			if (!plugin->GetAuthor().empty()) {
				plg::print(
				    "  {:<15} {}",
				    Colorize("Author:", Colors::GRAY),
				    Colorize(plugin->GetAuthor(), Colors::MAGENTA)
				);
			}
			if (!plugin->GetWebsite().empty()) {
				plg::print(
				    "  {:<15} {}",
				    Colorize("Website:", Colors::GRAY),
				    Colorize(plugin->GetWebsite(), Colors::BLUE)
				);
			}
			if (!plugin->GetLicense().empty()) {
				plg::print("  {:<15} {}", Colorize("License:", Colors::GRAY), plugin->GetLicense());
			}
		}

		// Plugin-specific information
		if (!plugin->GetEntry().empty()) {
			plg::print(Colorize("\n[Plugin Details]", Colors::CYAN));
			plg::print(
			    "  {:<15} {}",
			    Colorize("Entry Point:", Colors::GRAY),
			    Colorize(plugin->GetEntry(), Colors::YELLOW)
			);
		}

		// Methods
		const auto& methods = plugin->GetMethods();
		if (!methods.empty()) {
			plg::print(
			    Colorize("\n[Exported Methods]", Colors::CYAN)
			    + Colorize(std::format(" ({} total)", methods.size()), Colors::GRAY)
			);

			size_t displayCount = std::min<size_t>(methods.size(), 10);
			for (size_t i = 0; i < displayCount; ++i) {
				const auto& method = methods[i];
				plg::print(
				    "  {}{:<2} {}",
				    Colorize(Icons.Number, Colors::GRAY),
				    i + 1,
				    Colorize(method.GetName(), Colors::GREEN)
				);
				if (!method.GetFuncName().empty()) {
					plg::print(
					    "      {} {}",
					    Colorize("Func Name:", Colors::GRAY),
					    method.GetFuncName()
					);
				}
			}
			if (methods.size() > 10) {
				plg::print(
				    "  {} ... and {} more methods",
				    Colorize(Icons.Arrow, Colors::GRAY),
				    methods.size() - 10
				);
			}
		}

		// Platforms
		const auto& platforms = plugin->GetPlatforms();
		if (!platforms.empty()) {
			plg::print(Colorize("\n[Supported Platforms]", Colors::CYAN));
			plg::print("  {}", Colorize(plg::join(platforms, ", "), Colors::GREEN));
		}

		// Dependencies
		const auto& deps = plugin->GetDependencies();
		if (!deps.empty()) {
			plg::print(
			    Colorize("\n[Dependencies]", Colors::CYAN)
			    + Colorize(std::format(" ({} total)", deps.size()), Colors::GRAY)
			);
			for (const auto& dep : deps) {
				std::string depIndicator = dep.IsOptional() ? Colorize(Icons.Skipped, Colors::GRAY)
				                                            : Colorize(Icons.Valid, Colors::GREEN);
				plg::print(
				    "  {} {} {}",
				    depIndicator,
				    Colorize(dep.GetName(), Colors::BOLD),
				    Colorize(dep.GetConstraints().to_string(), Colors::GRAY)
				);
				if (dep.IsOptional()) {
					plg::print("    └─ {}", Colorize("Optional", Colors::GRAY));
				}
			}
		}

		// Conflicts
		const auto& conflicts = plugin->GetConflicts();
		if (!conflicts.empty()) {
			plg::print(
			    Colorize("\n[Conflicts]", Colors::YELLOW)
			    + Colorize(std::format(" ({} total)", conflicts.size()), Colors::GRAY)
			);
			for (const auto& conflict : conflicts) {
				plg::print(
				    "  {} {} {}",
				    Colorize(Icons.Warning, Colors::YELLOW),
				    conflict.GetName(),
				    Colorize(conflict.GetConstraints().to_string(), Colors::GRAY)
				);
				if (!conflict.GetReason().empty()) {
					plg::print("    └─ {}", Colorize(conflict.GetReason(), Colors::RED));
				}
			}
		}

		// Performance Information
		plg::print(Colorize("\n[Performance Metrics]", Colors::CYAN));
		auto totalTime = plugin->GetTotalTime();
		plg::print(
		    "  {:<15} {}",
		    Colorize("Total Time:", Colors::GRAY),
		    Colorize(
		        FormatDuration(totalTime),
		        totalTime > std::chrono::seconds(1) ? Colors::YELLOW : Colors::GREEN
		    )
		);

		// Show timing for different operations
		ExtensionState operations[] = {
			ExtensionState::Parsing,
			ExtensionState::Resolving,
			ExtensionState::Loading,
			ExtensionState::Starting,
		};

		for (const auto& op : operations) {
			try {
				auto duration = plugin->GetOperationTime(op);
				if (duration.count() > 0) {
					bool slow = duration > std::chrono::milliseconds(500);
					plg::print(
					    "  {:<15} {}",
					    Colorize(std::format("{}:", plg::enum_to_string(op)), Colors::GRAY),
					    Colorize(FormatDuration(duration), slow ? Colors::YELLOW : Colors::GREEN)
					);
				}
			} catch (...) {
			}
		}

		// Errors and Warnings
		if (plugin->HasErrors() || plugin->HasWarnings()) {
			plg::print(Colorize("\n[Issues]", Colors::RED));
			for (const auto& error : plugin->GetErrors()) {
				plg::print("  {} {}", Colorize("ERROR:", Colors::RED), error);
			}
			for (const auto& warning : plugin->GetWarnings()) {
				plg::print("  {} {}", Colorize("WARNING:", Colors::YELLOW), warning);
			}
		} else {
			plg::print(
			    "\n{} {}",
			    Colorize(Icons.Ok, Colors::GREEN),
			    Colorize("No issues detected", Colors::GREEN)
			);
		}

		plg::print(DOUBLE_LINE);
	}

	void ShowModule(std::string_view identifier, bool module_use_id, bool jsonOutput) {
		if (!CheckManager()) {
			return;
		}

		const auto& manager = plug->GetManager();
		auto module = module_use_id ? manager.FindExtension(FormatId(identifier))
		                            : manager.FindExtension(identifier);

		if (!module) {
			if (jsonOutput) {
				glz::json_t error;
				error["error"] = std::format("Module {} not found", identifier);
				plg::print(*error.dump());
			} else {
				plg::print("{}: Module {} not found.", Colorize("Error", Colors::RED), identifier);
			}
			return;
		}

		if (!module->IsModule()) {
			if (jsonOutput) {
				glz::json_t error;
				error["error"] = std::format("'{}' is not a module (it's a plugin)", identifier);
				plg::print(*error.dump());
			} else {
				plg::print(
				    "{}: '{}' is not a module (it's a plugin).",
				    Colorize("Error", Colors::RED),
				    identifier
				);
			}
			return;
		}

		// JSON output
		if (jsonOutput) {
			plg::print(*ExtensionToJson(module).dump());
			return;
		}

		// Display detailed module information with colors
		plg::print(DOUBLE_LINE);
		plg::print(
		    "{}: {}",
		    Colorize("MODULE INFORMATION", Colors::BOLD),
		    Colorize(module->GetName(), Colors::MAGENTA)
		);
		plg::print(DOUBLE_LINE);

		// Status indicator
		auto [symbol, stateColor] = GetStateInfo(module->GetState());
		plg::print(
		    "\n{} {} {}",
		    Colorize(symbol, stateColor),
		    Colorize("Status:", Colors::BOLD),
		    Colorize(plg::enum_to_string(module->GetState()), stateColor)
		);

		// Basic Information
		plg::print(Colorize("\n[Basic Information]", Colors::CYAN));
		plg::print("  {:<15} {}", Colorize("ID:", Colors::GRAY), module->GetId());
		plg::print("  {:<15} {}", Colorize("Name:", Colors::GRAY), module->GetName());
		plg::print(
		    "  {:<15} {}",
		    Colorize("Version:", Colors::GRAY),
		    Colorize(module->GetVersionString(), Colors::GREEN)
		);
		plg::print("  {:<15} {}", Colorize("Language:", Colors::GRAY), module->GetLanguage());
		plg::print(
		    "  {:<15} {}",
		    Colorize("Location:", Colors::GRAY),
		    plg::as_string(module->GetLocation())
		);
		plg::print(
		    "  {:<15} {}",
		    Colorize("File Size:", Colors::GRAY),
		    FormatFileSize(module->GetLocation())
		);

		// Optional Information
		if (!module->GetDescription().empty() || !module->GetAuthor().empty()
		    || !module->GetWebsite().empty() || !module->GetLicense().empty()) {
			plg::print(Colorize("\n[Additional Information]", Colors::CYAN));
			if (!module->GetDescription().empty()) {
				plg::print(
				    "  {:<15} {}",
				    Colorize("Description:", Colors::GRAY),
				    module->GetDescription()
				);
			}
			if (!module->GetAuthor().empty()) {
				plg::print(
				    "  {:<15} {}",
				    Colorize("Author:", Colors::GRAY),
				    Colorize(module->GetAuthor(), Colors::MAGENTA)
				);
			}
			if (!module->GetWebsite().empty()) {
				plg::print(
				    "  {:<15} {}",
				    Colorize("Website:", Colors::GRAY),
				    Colorize(module->GetWebsite(), Colors::BLUE)
				);
			}
			if (!module->GetLicense().empty()) {
				plg::print("  {:<15} {}", Colorize("License:", Colors::GRAY), module->GetLicense());
			}
		}

		// Module-specific information
		if (!module->GetRuntime().empty()) {
			plg::print(Colorize("\n[Module Details]", Colors::CYAN));
			plg::print(
			    "  {:<15} {}",
			    Colorize("Runtime:", Colors::GRAY),
			    Colorize(plg::as_string(module->GetRuntime()), Colors::YELLOW)
			);
		}

		// Directories
		const auto& dirs = module->GetDirectories();
		if (!dirs.empty()) {
			plg::print(
			    Colorize("\n[Search Directories]", Colors::CYAN)
			    + Colorize(std::format(" ({} total)", dirs.size()), Colors::GRAY)
			);

			size_t displayCount = std::min<size_t>(dirs.size(), 5);
			for (size_t i = 0; i < displayCount; ++i) {
				bool exists = std::filesystem::exists(dirs[i]);
				plg::print(
				    "  {} {}",
				    exists ? Colorize(Icons.Ok, Colors::GREEN) : Colorize(Icons.Fail, Colors::RED),
				    plg::as_string(dirs[i])
				);
			}
			if (dirs.size() > 5) {
				plg::print(
				    "  {} ... and {} more directories",
				    Colorize(Icons.Arrow, Colors::GRAY),
				    dirs.size() - 5
				);
			}
		}

		// Assembly Information
		if (auto assembly = module->GetAssembly()) {
			plg::print(Colorize("\n[Assembly Information]", Colors::CYAN));
			plg::print("  {} Assembly loaded and active", Colorize(Icons.Ok, Colors::GREEN));
			// Add more assembly details if available in your IAssembly interface
		}

		// Platforms
		const auto& platforms = module->GetPlatforms();
		if (!platforms.empty()) {
			plg::print(Colorize("\n[Supported Platforms]", Colors::CYAN));
			plg::print("  {}", Colorize(plg::join(platforms, ", "), Colors::GREEN));
		}

		// Dependencies
		const auto& deps = module->GetDependencies();
		if (!deps.empty()) {
			plg::print(
			    Colorize("\n[Dependencies]", Colors::CYAN)
			    + Colorize(std::format(" ({} total)", deps.size()), Colors::GRAY)
			);
			for (const auto& dep : deps) {
				std::string depIndicator = dep.IsOptional() ? Colorize(Icons.Skipped, Colors::GRAY)
				                                            : Colorize(Icons.Valid, Colors::GREEN);
				plg::print(
				    "  {} {} {}",
				    depIndicator,
				    Colorize(dep.GetName(), Colors::BOLD),
				    Colorize(dep.GetConstraints().to_string(), Colors::GRAY)
				);
				if (dep.IsOptional()) {
					plg::print("    └─ {}", Colorize("Optional", Colors::GRAY));
				}
			}
		}

		// Conflicts
		const auto& conflicts = module->GetConflicts();
		if (!conflicts.empty()) {
			plg::print(
			    Colorize("\n[Conflicts]", Colors::YELLOW)
			    + Colorize(std::format(" ({} total)", conflicts.size()), Colors::GRAY)
			);
			for (const auto& conflict : conflicts) {
				plg::print(
				    "  {} {} {}",
				    Colorize(Icons.Warning, Colors::YELLOW),
				    conflict.GetName(),
				    Colorize(conflict.GetConstraints().to_string(), Colors::GRAY)
				);
				if (!conflict.GetReason().empty()) {
					plg::print("    └─ {}", Colorize(conflict.GetReason(), Colors::RED));
				}
			}
		}

		// Performance Information
		plg::print(Colorize("\n[Performance Metrics]", Colors::CYAN));
		auto totalTime = module->GetTotalTime();
		plg::print(
		    "  {:<15} {}",
		    Colorize("Total Time:", Colors::GRAY),
		    Colorize(
		        FormatDuration(totalTime),
		        totalTime > std::chrono::seconds(1) ? Colors::YELLOW : Colors::GREEN
		    )
		);

		// Show timing for different operations
		ExtensionState operations[] = {
			ExtensionState::Parsing,
			ExtensionState::Resolving,
			ExtensionState::Loading,
			ExtensionState::Starting,
		};

		for (const auto& op : operations) {
			try {
				auto duration = module->GetOperationTime(op);
				if (duration.count() > 0) {
					bool slow = duration > std::chrono::milliseconds(500);
					plg::print(
					    "  {:<15} {}",
					    Colorize(std::format("{}:", plg::enum_to_string(op)), Colors::GRAY),
					    Colorize(FormatDuration(duration), slow ? Colors::YELLOW : Colors::GREEN)
					);
				}
			} catch (...) {
			}
		}

		// Errors and Warnings
		if (module->HasErrors() || module->HasWarnings()) {
			plg::print(Colorize("\n[Issues]", Colors::RED));
			for (const auto& error : module->GetErrors()) {
				plg::print("  {} {}", Colorize("ERROR:", Colors::RED), error);
			}
			for (const auto& warning : module->GetWarnings()) {
				plg::print("  {} {}", Colorize("WARNING:", Colors::YELLOW), warning);
			}
		} else {
			plg::print(
			    "\n{} {}",
			    Colorize(Icons.Ok, Colors::GREEN),
			    Colorize("No issues detected", Colors::GREEN)
			);
		}

		plg::print(DOUBLE_LINE);
	}

	void ShowHealth() {
		if (!CheckManager()) {
			return;
		}

		const auto& manager = plug->GetManager();
		auto report = CalculateSystemHealth(manager);

		// Determine health status color
		std::string_view scoreColor = Colors::GREEN;
		std::string status = "HEALTHY";
		if (report.score < 50) {
			scoreColor = Colors::RED;
			status = "CRITICAL";
		} else if (report.score < 75) {
			scoreColor = Colors::YELLOW;
			status = "WARNING";
		}

		plg::print(DOUBLE_LINE);
		plg::print(Colorize("SYSTEM HEALTH CHECK", Colors::BOLD));
		plg::print(DOUBLE_LINE);

		// Overall score
		plg::print(
		    "\n{}: {} {}",
		    Colorize("Overall Health Score", Colors::BOLD),
		    Colorize(std::format("{}/100", report.score), scoreColor),
		    Colorize(std::format("[{}]", status), scoreColor)
		);

		// Statistics
		plg::print(Colorize("\n[Statistics]", Colors::CYAN));
		plg::print("  Total Extensions:        {}", report.statistics["total_extensions"]);
		plg::print(
		    "  Failed Extensions:       {} {}",
		    report.statistics["failed_extensions"],
		    report.statistics["failed_extensions"] > 0 ? Colorize(Icons.Warning, Colors::RED)
		                                               : Colorize(Icons.Ok, Colors::GREEN)
		);
		plg::print(
		    "  Extensions with Errors:  {} {}",
		    report.statistics["extensions_with_errors"],
		    report.statistics["extensions_with_errors"] > 0 ? Colorize(Icons.Warning, Colors::YELLOW)
		                                                    : Colorize(Icons.Ok, Colors::GREEN)
		);
		plg::print("  Total Warnings:          {}", report.statistics["total_warnings"]);
		plg::print("  Slow Loading Extensions: {}", report.statistics["slow_loading_extensions"]);

		// Issues
		if (!report.issues.empty()) {
			plg::print(Colorize("\n[Critical Issues]", Colors::RED));
			for (const auto& issue : report.issues) {
				plg::print("  {} {}", Colorize(Icons.Fail, Colors::RED), issue);
			}
		}

		// Warnings
		if (!report.warnings.empty()) {
			plg::print(Colorize("\n[Warnings]", Colors::YELLOW));
			for (const auto& warning : report.warnings) {
				plg::print("  {} {}", Colorize(Icons.Warning, Colors::YELLOW), warning);
			}
		}

		// Recommendations
		plg::print(Colorize("\n[Recommendations]", Colors::CYAN));
		if (report.score == 100) {
			plg::print("  {} System is running optimally!", Colorize(Icons.Ok, Colors::GREEN));
		} else {
			if (report.statistics["failed_extensions"] > 0) {
				plg::print("  • Fix or remove failed extensions");
			}
			if (report.statistics["extensions_with_errors"] > 0) {
				plg::print("  • Review and resolve extension errors");
			}
			if (report.statistics["slow_loading_extensions"] > 0) {
				plg::print("  • Investigate slow-loading extensions for optimization");
			}
		}

		plg::print(DOUBLE_LINE);
	}

	void ShowDependencyTree(std::string_view name, bool useId = false) {
		if (!CheckManager()) {
			return;
		}

		const auto& manager = plug->GetManager();
		auto ext = useId ? manager.FindExtension(FormatId(name)) : manager.FindExtension(name);

		if (!ext) {
			plg::print("{} {} not found.", Colorize("Error:", Colors::RED), name);
			return;
		}

		plg::print(DOUBLE_LINE);
		plg::print("{}: {}", Colorize("DEPENDENCY TREE", Colors::BOLD), ext->GetName());
		plg::print(DOUBLE_LINE);
		plg::print("");

		PrintDependencyTree(ext, manager);

		// Also show what depends on this extension
		plg::print(Colorize("\n[Reverse Dependencies]", Colors::CYAN));
		plg::print("Extensions that depend on this:");

		bool found = false;
		for (const auto& other : manager.GetExtensions()) {
			for (const auto& dep : other->GetDependencies()) {
				if (dep.GetName() == ext->GetName()) {
					plg::print(
					    "  • {} {}",
					    other->GetName(),
					    dep.IsOptional() ? Colorize("[optional]", Colors::GRAY) : ""
					);
					found = true;
				}
			}
		}

		if (!found) {
			plg::print("  {}", Colorize("None", Colors::GRAY));
		}

		plg::print(DOUBLE_LINE);
	}

	void SearchExtensions(std::string_view query) {
		if (!CheckManager()) {
			return;
		}

		const auto& manager = plug->GetManager();
		auto allExtensions = manager.GetExtensions();

		std::vector<const Extension*> matches;
		std::string lowerQuery(query);
		std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

		for (const auto& ext : allExtensions) {
			std::string name = ext->GetName();
			std::string desc = ext->GetDescription();
			std::string author = ext->GetAuthor();

			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);
			std::transform(author.begin(), author.end(), author.begin(), ::tolower);

			if (name.find(lowerQuery) != std::string::npos || desc.find(lowerQuery) != std::string::npos
			    || author.find(lowerQuery) != std::string::npos) {
				matches.push_back(ext);
			}
		}

		if (matches.empty()) {
			plg::print(
			    "{} No extensions found matching '{}'",
			    Colorize(Icons.Missing, Colors::YELLOW),
			    query
			);
			return;
		}

		plg::print(
		    "{}: Found {} match{} for '{}'",
		    Colorize("SEARCH RESULTS", Colors::BOLD),
		    matches.size(),
		    matches.size() > 1 ? "es" : "",
		    query
		);
		plg::print(SEPARATOR_LINE);

		for (const auto& ext : matches) {
			auto [symbol, color] = GetStateInfo(ext->GetState());
			plg::print(
			    "{} {} {} {} {}",
			    Colorize(symbol, color),
			    Colorize(ext->GetName(), Colors::BOLD),
			    Colorize(ext->GetVersionString(), Colors::GRAY),
			    ext->IsPlugin() ? "[Plugin]" : "[Module]",
			    Colorize(std::format("({})", ext->GetLanguage()), Colors::GRAY)
			);

			if (!ext->GetDescription().empty()) {
				plg::print("  {}", Truncate(ext->GetDescription(), 70));
			}
		}
		plg::print(SEPARATOR_LINE);
	}

	void ValidateExtension(const std::filesystem::path& path) {
		plg::print("{}: {}", Colorize("VALIDATING", Colors::BOLD), plg::as_string(path));
		plg::print(SEPARATOR_LINE);

		// Check if file exists
		if (!std::filesystem::exists(path)) {
			plg::print("{} File does not exist", Colorize("✗", Colors::RED));
			return;
		}

		// Check file extension
		const auto& fileExt = plg::as_string(path.extension());
		bool isPlugin = (fileExt == ".plg" || fileExt == ".pplugin");
		bool isModule = (fileExt == ".mod" || fileExt == ".pmodule");

		if (!isPlugin && !isModule) {
			plg::print("{} Invalid file extension: {}", Colorize("✗", Colors::RED), fileExt);
			return;
		}

		plg::print("{} File exists", Colorize(Icons.Ok, Colors::GREEN));
		plg::print(
		    "{} Valid extension type: {}",
		    Colorize(Icons.Ok, Colors::GREEN),
		    isPlugin ? "Plugin" : "Module"
		);
		plg::print("{} File size: {}", Colorize(Icons.Missing, Colors::CYAN), FormatFileSize(path));

		// Try to parse manifest (you'd need to implement manifest parsing)
		// This is a placeholder for actual validation logic
		/*plg::print("{} Manifest validation: {}",
		            Colorize(Icons.Missing, Colors::CYAN),
		            "Would parse and validate manifest here");*/
		// TODO

		plg::print(SEPARATOR_LINE);
		plg::print("{}: Validation complete", Colorize("RESULT", Colors::BOLD));
	}

	void CompareExtensions(std::string_view name1, std::string_view name2, bool useId = false) {
		if (!CheckManager()) {
			return;
		}

		const auto& manager = plug->GetManager();
		auto ext1 = useId ? manager.FindExtension(FormatId(name1)) : manager.FindExtension(name1);
		auto ext2 = useId ? manager.FindExtension(FormatId(name2)) : manager.FindExtension(name2);

		if (!ext1) {
			plg::print("{}: Extension {} not found.", Colorize("Error", Colors::RED), name1);
			return;
		}
		if (!ext2) {
			plg::print("{}: Extension {} not found.", Colorize("Error", Colors::RED), name2);
			return;
		}

		plg::print(DOUBLE_LINE);
		plg::print(Colorize("EXTENSION COMPARISON", Colors::BOLD));
		plg::print(DOUBLE_LINE);

		// Basic comparison table
		auto printRow = [](std::string_view label, std::string_view val1, std::string_view val2) {
			bool same = (val1 == val2);
			plg::print("{:<20} {:<25} {} {:<25}", label, val1, same ? Icons.Equal : Icons.NotEqual, val2);
		};

		plg::print(
		    "\n{:<20} {:<25}   {:<25}",
		    "",
		    Colorize(ext1->GetName(), Colors::CYAN),
		    Colorize(ext2->GetName(), Colors::MAGENTA)
		);
		plg::print(SEPARATOR_LINE);

		printRow("Type:", ext1->IsPlugin() ? "Plugin" : "Module", ext2->IsPlugin() ? "Plugin" : "Module");
		printRow("Version:", ext1->GetVersionString(), ext2->GetVersionString());
		printRow("Language:", ext1->GetLanguage(), ext2->GetLanguage());
		printRow("State:", plg::enum_to_string(ext1->GetState()), plg::enum_to_string(ext2->GetState()));
		printRow("Author:", ext1->GetAuthor(), ext2->GetAuthor());
		printRow("License:", ext1->GetLicense(), ext2->GetLicense());

		// Dependencies comparison
		plg::print(Colorize("\n[Dependencies]", Colors::BOLD));
		auto deps1 = ext1->GetDependencies();
		auto deps2 = ext2->GetDependencies();

		std::set<std::string> depNames1, depNames2;
		for (const auto& d : deps1) {
			depNames1.insert(d.GetName());
		}
		for (const auto& d : deps2) {
			depNames2.insert(d.GetName());
		}

		std::vector<std::string> onlyIn1, onlyIn2, common;
		std::set_difference(
		    depNames1.begin(),
		    depNames1.end(),
		    depNames2.begin(),
		    depNames2.end(),
		    std::back_inserter(onlyIn1)
		);
		std::set_difference(
		    depNames2.begin(),
		    depNames2.end(),
		    depNames1.begin(),
		    depNames1.end(),
		    std::back_inserter(onlyIn2)
		);
		std::set_intersection(
		    depNames1.begin(),
		    depNames1.end(),
		    depNames2.begin(),
		    depNames2.end(),
		    std::back_inserter(common)
		);

		if (!common.empty()) {
			plg::print("  Common: {}", plg::join(common, ", "));
		}
		if (!onlyIn1.empty()) {
			plg::print("  Only in {}: {}", ext1->GetName(), plg::join(onlyIn1, ", "));
		}
		if (!onlyIn2.empty()) {
			plg::print("  Only in {}: {}", ext2->GetName(), plg::join(onlyIn2, ", "));
		}

		// Performance comparison
		plg::print(Colorize("\n[Performance]", Colors::BOLD));
		plg::print(
		    "  Load Time:     {:<15} vs {:<15}",
		    FormatDuration(ext1->GetOperationTime(ExtensionState::Loaded)),
		    FormatDuration(ext2->GetOperationTime(ExtensionState::Loaded))
		);
		plg::print(
		    "  Total Time:    {:<15} vs {:<15}",
		    FormatDuration(ext1->GetTotalTime()),
		    FormatDuration(ext2->GetTotalTime())
		);

		plg::print(DOUBLE_LINE);
	}

	void Update() {
		if (plug->IsInitialized()) {
			plug->Update();
		}
	}

	bool IsInitialized() const {
		return plug->IsInitialized();
	}

private:
	bool CheckManager() const {
		if (!plug->IsInitialized()) {
			plg::print("{}: Initialize system before use.", Colorize("Error", Colors::RED));
			return false;
		}
		const auto& manager = plug->GetManager();
		if (!manager.IsInitialized()) {
			plg::print(
			    "{}: You must load plugin manager before query any information from it.",
			    Colorize("Error", Colors::RED)
			);
			return false;
		}
		return true;
	}

	std::shared_ptr<Plugify> plug;
};

// Enhanced interactive mode
void RunEnhancedInteractiveMode(PlugifyApp& app) {
	plg::print(
	    Colorize("Entering interactive mode. Type 'help' for commands or 'exit' to quit.", Colors::CYAN)
	);

	// Show prompt with color
	auto getPrompt = [&app]() -> std::string {
		if (g_useColor) {
			return app.IsInitialized() ? std::format("{}plugify>{} ", Colors::GREEN, Colors::RESET)
			                           : std::format("{}plugify>{} ", Colors::YELLOW, Colors::RESET);
		}
		return "plugify> ";
	};

	std::string line;
	while (true) {
		std::cout << getPrompt() << std::flush;
		if (!std::getline(std::cin, line)) {
			break;
		}
		if (line.empty()) {
			continue;
		}

		// Parse the line
		std::vector<std::string> args;
		std::istringstream iss(line);
		std::string word;
		while (iss >> word) {
			args.push_back(word);
		}

		if (args.empty()) {
			continue;
		}

		// Handle exit commands
		if (args[0] == "exit" || args[0] == "quit" || args[0] == "q") {
			plg::print(Colorize("Goodbye!", Colors::CYAN));
			break;
		}

		// Handle clear command
		if (args[0] == "clear" || args[0] == "cls") {
#ifdef _WIN32
			system("cls");
#else
			system("clear");
#endif
			continue;
		}

		// Create a temporary CLI app for parsing
		CLI::App interactiveApp{ "Interactive command" };
		interactiveApp.require_subcommand();  // 1 or more
		// interactiveApp.allow_extras();
		// interactiveApp.prefix_command();
		interactiveApp.set_version_flag("-v,--version", app.GetVersionString());
		interactiveApp.usage("Usage: plugify <command> [options]");
		// flag to display full help at once
		interactiveApp.set_help_flag();
		interactiveApp.set_help_all_flag("-h, --help", "Print this help message and exit");

		// Add all commands (similar to main but simplified)
		auto* init = interactiveApp.add_subcommand("init", "Initialize");
		auto* term = interactiveApp.add_subcommand("term", "Terminate");
		auto* load = interactiveApp.add_subcommand("load", "Load manager");
		auto* unload = interactiveApp.add_subcommand("unload", "Unload manager");
		auto* reload = interactiveApp.add_subcommand("reload", "Reload manager");
		auto* plugins = interactiveApp.add_subcommand("plugins", "List plugins");
		auto* modules = interactiveApp.add_subcommand("modules", "List modules");
		// Show plugin/module commands
		auto* plugin = interactiveApp.add_subcommand("plugin", "Show plugin information");
		auto* module = interactiveApp.add_subcommand("module", "Show module information");
		auto* health = interactiveApp.add_subcommand("health", "System health");
		auto* tree = interactiveApp.add_subcommand("tree", "Show dependency tree");
		auto* search = interactiveApp.add_subcommand("search", "Search extensions");
		auto* validate = interactiveApp.add_subcommand("validate", "Validate extension file");
		auto* compare = interactiveApp.add_subcommand("compare", "Compare two extensions");

		// Enhanced list commands with filters and sorting
		std::string pluginFilterState;
		std::string pluginFilterLang;
		std::string pluginSortBy = "name";
		bool pluginReverse = false;
		bool pluginShowFailed = false;

		plugins->add_option(
		    "--filter-state",
		    pluginFilterState,
		    "Filter by state (comma-separated: loaded,failed,disabled)"
		);
		plugins->add_option(
		    "--filter-lang",
		    pluginFilterLang,
		    "Filter by language (comma-separated: cpp,python,rust)"
		);
		plugins
		    ->add_option("-s,--sort", pluginSortBy, "Sort by: name, version, state, language, loadtime")
		    ->check(CLI::IsMember({ "name", "version", "state", "language", "loadtime" }));
		plugins->add_flag("-r,--reverse", pluginReverse, "Reverse sort order");
		plugins->add_flag("-f,--failed", pluginShowFailed, "Show only failed plugins");

		std::string moduleFilterState;
		std::string moduleFilterLang;
		std::string moduleSortBy = "name";
		bool moduleReverse = false;
		bool moduleShowFailed = false;

		modules->add_option("--filter-state", moduleFilterState, "Filter by state (comma-separated)");
		modules->add_option("--filter-lang", moduleFilterLang, "Filter by language (comma-separated)");
		modules
		    ->add_option("-s,--sort", moduleSortBy, "Sort by: name, version, state, language, loadtime")
		    ->check(CLI::IsMember({ "name", "version", "state", "language", "loadtime" }));
		modules->add_flag("-r,--reverse", moduleReverse, "Reverse sort order");
		modules->add_flag("-f,--failed", moduleShowFailed, "Show only failed modules");

		// Add options for plugin/module
		std::string plugin_name;
		bool plugin_use_id = false;
		plugin->add_option("name", plugin_name, "Plugin name or ID")->required();
		plugin->add_flag("-u,--uuid", plugin_use_id, "Use ID instead of name");
		plugin->validate_positionals();

		std::string module_name;
		bool module_use_id = false;
		module->add_option("name", module_name, "Module name or ID")->required();
		module->add_flag("-u,--uuid", module_use_id, "Use ID instead of name");
		module->validate_positionals();

		std::string tree_name;
		bool tree_use_id = false;
		tree->add_option("name", tree_name, "Extension name or ID")->required();
		tree->add_flag("-u,--uuid", tree_use_id, "Use ID instead of name");
		tree->validate_positionals();

		std::string search_query;
		search->add_option("query", search_query, "Search query")->required();
		search->validate_positionals();

		std::string validate_path;
		validate->add_option("path", validate_path, "Path to extension file")->required();
		validate->validate_positionals();

		std::string compare_ext1, compare_ext2;
		bool compare_use_id = false;
		compare->add_option("extension1", compare_ext1, "First extension")->required();
		compare->add_option("extension2", compare_ext2, "Second extension")->required();
		compare->add_flag("-u,--uuid", compare_use_id, "Use ID instead of name");
		compare->validate_positionals();

		// Set callbacks
		init->callback([&app]() { app.Initialize(); });
		term->callback([&app]() { app.Terminate(); });
		load->callback([&app]() { app.LoadManager(); });
		unload->callback([&app]() { app.UnloadManager(); });
		reload->callback([&app]() { app.ReloadManager(); });

		plugins->callback([&]() {
			FilterOptions filter;
			if (!pluginFilterState.empty()) {
				filter.states = ParseStates(ParseCsv(pluginFilterState));
			}
			if (!pluginFilterLang.empty()) {
				filter.languages = ParseCsv(pluginFilterLang);
			}
			filter.showOnlyFailed = pluginShowFailed;

			app.ListPlugins(filter, ParseSortBy(pluginSortBy), pluginReverse, false);
		});

		modules->callback([&]() {
			FilterOptions filter;
			if (!moduleFilterState.empty()) {
				filter.states = ParseStates(ParseCsv(moduleFilterState));
			}
			if (!moduleFilterLang.empty()) {
				filter.languages = ParseCsv(moduleFilterLang);
			}
			filter.showOnlyFailed = moduleShowFailed;

			app.ListModules(filter, ParseSortBy(moduleSortBy), moduleReverse, false);
		});

		plugin->callback([&app, &plugin_name, &plugin_use_id]() {
			app.ShowPlugin(plugin_name, plugin_use_id, false);
		});

		module->callback([&app, &module_name, &module_use_id]() {
			app.ShowModule(module_name, module_use_id, false);
		});

		health->callback([&app]() { app.ShowHealth(); });

		tree->callback([&app, &tree_name, &tree_use_id]() {
			app.ShowDependencyTree(tree_name, tree_use_id);
		});

		search->callback([&app, &search_query]() {
			if (!search_query.empty()) {
				app.SearchExtensions(search_query);
			} else {
				plg::print("Search query required");
			}
		});

		validate->callback([&app, &validate_path]() { app.ValidateExtension(validate_path); });

		compare->callback([&app, &compare_ext1, &compare_ext2, &compare_use_id]() {
			app.CompareExtensions(compare_ext1, compare_ext2, compare_use_id);
		});

		try {
			interactiveApp.parse(args);
		} catch (const CLI::ParseError& e) {
			interactiveApp.exit(e);
		}

		app.Update();
	}
}

// Updated main.cpp with enhanced CLI features

int main(int argc, char** argv) {
	/*if (!std::setlocale(LC_ALL, "en_US.UTF-8")) {
	    plg::error("Warning: Failed to set UTF-8 locale");
	}*/

	auto plugify = MakePlugify();
	if (!plugify) {
		plg::print("{}: {}", Colorize("Error", Colors::RED), plugify.error());
		return EXIT_FAILURE;
	}

	PlugifyApp app(std::move(*plugify));

	CLI::App cliApp{ "Plugify - Plugin Management System" };
	//cliApp.require_subcommand();  // 1 or more
	cliApp.set_version_flag("-v,--version", app.GetVersionString());
	cliApp.usage("Usage: plugify <command> [options]");
	// flag to display full help at once
	cliApp.set_help_flag("");
	cliApp.set_help_all_flag("-h, --help");

	// Global options
	bool interactive = false;
	bool noColor = false;
	bool jsonOutput = false;

	cliApp.add_flag("-i,--interactive", interactive, "Run in interactive mode");
	cliApp.add_flag("-p,--no-color", noColor, "Disable colored output");
	cliApp.add_flag("-j,--json", jsonOutput, "Output in JSON format");

	// Define subcommands
	auto* init_cmd = cliApp.add_subcommand("init", "Initialize plugin system");
	auto* term_cmd = cliApp.add_subcommand("term", "Terminate plugin system");
	auto* load_cmd = cliApp.add_subcommand("load", "Load plugin manager");
	auto* unload_cmd = cliApp.add_subcommand("unload", "Unload plugin manager");
	auto* reload_cmd = cliApp.add_subcommand("reload", "Reload plugin manager");

	// Enhanced list commands with filters and sorting
	auto* plugins_cmd = cliApp.add_subcommand("plugins", "List all plugins");
	std::string pluginFilterState;
	std::string pluginFilterLang;
	std::string pluginSortBy = "name";
	bool pluginReverse = false;
	bool pluginShowFailed = false;

	plugins_cmd->add_option(
	    "--filter-state",
	    pluginFilterState,
	    "Filter by state (comma-separated: loaded,failed,disabled)"
	);
	plugins_cmd->add_option(
	    "--filter-lang",
	    pluginFilterLang,
	    "Filter by language (comma-separated: cpp,python,rust)"
	);
	plugins_cmd
	    ->add_option("-s,--sort", pluginSortBy, "Sort by: name, version, state, language, loadtime")
	    ->check(CLI::IsMember({ "name", "version", "state", "language", "loadtime" }));
	plugins_cmd->add_flag("-r,--reverse", pluginReverse, "Reverse sort order");
	plugins_cmd->add_flag("-f,--failed", pluginShowFailed, "Show only failed plugins");

	auto* modules_cmd = cliApp.add_subcommand("modules", "List all modules");
	std::string moduleFilterState;
	std::string moduleFilterLang;
	std::string moduleSortBy = "name";
	bool moduleReverse = false;
	bool moduleShowFailed = false;

	modules_cmd->add_option("--filter-state", moduleFilterState, "Filter by state (comma-separated)");
	modules_cmd->add_option("--filter-lang", moduleFilterLang, "Filter by language (comma-separated)");
	modules_cmd
	    ->add_option("-s,--sort", moduleSortBy, "Sort by: name, version, state, language, loadtime")
	    ->check(CLI::IsMember({ "name", "version", "state", "language", "loadtime" }));
	modules_cmd->add_flag("-r,--reverse", moduleReverse, "Reverse sort order");
	modules_cmd->add_flag("-f,--failed", moduleShowFailed, "Show only failed modules");

	// Show plugin/module commands
	auto* plugin_cmd = cliApp.add_subcommand("plugin", "Show plugin information");
	std::string plugin_name;
	bool plugin_use_id = false;
	plugin_cmd->add_option("name", plugin_name, "Plugin name or ID")->required();
	plugin_cmd->add_flag("-u,--uuid", plugin_use_id, "Use ID instead of name");
	plugin_cmd->validate_positionals();

	auto* module_cmd = cliApp.add_subcommand("module", "Show module information");
	std::string module_name;
	bool module_use_id = false;
	module_cmd->add_option("name", module_name, "Module name or ID")->required();
	module_cmd->add_flag("-u,--uuid", module_use_id, "Use ID instead of name");
	module_cmd->validate_positionals();

	// New commands
	auto* health_cmd = cliApp.add_subcommand("health", "Show system health report");

	auto* tree_cmd = cliApp.add_subcommand("tree", "Show dependency tree");
	std::string tree_name;
	bool tree_use_id = false;
	tree_cmd->add_option("name", tree_name, "Extension name or ID")->required();
	tree_cmd->add_flag("-u,--uuid", tree_use_id, "Use ID instead of name");
	tree_cmd->validate_positionals();

	auto* search_cmd = cliApp.add_subcommand("search", "Search extensions");
	std::string search_query;
	search_cmd->add_option("query", search_query, "Search query")->required();
	search_cmd->validate_positionals();

	auto* validate_cmd = cliApp.add_subcommand("validate", "Validate extension file");
	std::string validate_path;
	validate_cmd->add_option("path", validate_path, "Path to extension file")->required();
	validate_cmd->validate_positionals();

	auto* compare_cmd = cliApp.add_subcommand("compare", "Compare two extensions");
	std::string compare_ext1, compare_ext2;
	bool compare_use_id = false;
	compare_cmd->add_option("ex1", compare_ext1, "First extension")->required();
	compare_cmd->add_option("ex2", compare_ext2, "Second extension")->required();
	compare_cmd->validate_positionals();
	compare_cmd->add_flag("-u,--uuid", compare_use_id, "Use ID instead of name");

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

	plugins_cmd->callback([&]() {
		if (!app.IsInitialized()) {
			app.Initialize();
		}

		FilterOptions filter;
		if (!pluginFilterState.empty()) {
			filter.states = ParseStates(ParseCsv(pluginFilterState));
		}
		if (!pluginFilterLang.empty()) {
			filter.languages = ParseCsv(pluginFilterLang);
		}
		filter.showOnlyFailed = pluginShowFailed;

		app.ListPlugins(filter, ParseSortBy(pluginSortBy), pluginReverse, jsonOutput);
	});

	modules_cmd->callback([&]() {
		if (!app.IsInitialized()) {
			app.Initialize();
		}

		FilterOptions filter;
		if (!moduleFilterState.empty()) {
			filter.states = ParseStates(ParseCsv(moduleFilterState));
		}
		if (!moduleFilterLang.empty()) {
			filter.languages = ParseCsv(moduleFilterLang);
		}
		filter.showOnlyFailed = moduleShowFailed;

		app.ListModules(filter, ParseSortBy(moduleSortBy), moduleReverse, jsonOutput);
	});

	plugin_cmd->callback([&app, &plugin_name, &plugin_use_id, &jsonOutput]() {
		if (!app.IsInitialized()) {
			app.Initialize();
		}
		app.ShowPlugin(plugin_name, plugin_use_id, jsonOutput);
	});

	module_cmd->callback([&app, &module_name, &module_use_id, &jsonOutput]() {
		if (!app.IsInitialized()) {
			app.Initialize();
		}
		app.ShowModule(module_name, module_use_id, jsonOutput);
	});

	health_cmd->callback([&app]() {
		if (!app.IsInitialized()) {
			app.Initialize();
		}
		app.ShowHealth();
	});

	tree_cmd->callback([&app, &tree_name, &tree_use_id]() {
		if (!app.IsInitialized()) {
			app.Initialize();
		}
		app.ShowDependencyTree(tree_name, tree_use_id);
	});

	search_cmd->callback([&app, &search_query]() {
		if (!app.IsInitialized()) {
			app.Initialize();
		}
		app.SearchExtensions(search_query);
	});

	validate_cmd->callback([&app, &validate_path]() { app.ValidateExtension(validate_path); });

	compare_cmd->callback([&app, &compare_ext1, &compare_ext2, &compare_use_id]() {
		if (!app.IsInitialized()) {
			app.Initialize();
		}
		app.CompareExtensions(compare_ext1, compare_ext2, compare_use_id);
	});

	// Parse command line arguments
	try {
		// Set global color flag
		if (noColor) {
			g_useColor = false;
		}

// Check if terminal supports color (optional)
#ifdef _WIN32
// Windows terminal color detection
#else
		// Unix terminal color detection
		if (!isatty(fileno(stdout))) {
			g_useColor = false;
		}
#endif

		cliApp.parse(argc, argv);

		// If no subcommand was provided or interactive flag is set, run interactive mode
		if (interactive || argc == 1) {
			RunEnhancedInteractiveMode(app);
		}
	} catch (const CLI::ParseError& e) {
		return cliApp.exit(e);
	} catch (const std::exception& e) {
		plg::print("{}: {}", Colorize("Error", Colors::RED), e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// ============================================================================
// Usage Examples
// ============================================================================
/*
// Example 1: Simple usage with defaults
int main() {
    auto result = plugify::MakePlugify("./app");
    if (!result) {
        std::cerr << "Failed to create Plugify: " << result.error().message << std::endl;
        return 1;
    }

    auto plugify = result.value();
    if (auto initResult = plugify->Initialize(); !initResult) {
        std::cerr << "Failed to initialize: " << initResult.error().message << std::endl;
        return 1;
    }

    // Load extensions
    const auto& manager = plugify->GetManager();
    manager->Initialize();

    // Main loop
    bool running = true;
    while (running) {
        plugify->Update();
        // Your application logic here
    }

    plugify->Terminate();
    return 0;
}

// Example 2: Advanced configuration
int main() {
    // Create custom logger
    auto logger = std::make_shared<plugify::impl::AsyncLogger>(
        std::make_shared<plugify::impl::FileLogger>("./logs/app.log")
    );

    // Configure plugin manager
    plugify::Config config;
    config.paths.baseDir = "./app";
    config.paths.extensionsDir = "extensions";
    config.loading.enableHotReload = true;
    config.loading.parallelLoading = true;
    config.runtime.updateInterval = std::chrono::milliseconds(16);

    // Build Plugify instance
    auto result = plugify::Plugify::CreateBuilder()
        .WithConfig(config)
        .WithLogger(logger)
        .WithService<MyCustomService>(std::make_shared<MyCustomServiceImpl>())
        .Build();

    if (!result) {
        std::cerr << "Failed: " << result.error().message << std::endl;
        return 1;
    }

    auto plugify = result.value();

    // Initialize asynchronously
    auto initFuture = plugify->InitializeAsync();

    // Do other initialization...

    // Wait for initialization
    if (auto initResult = initFuture.get(); !initResult) {
        std::cerr << "Failed to initialize: " << initResult.error().message << std::endl;
        return 1;
    }

    // Access service locator for convenient operations
    auto logger = plugify->GetServices().Resolve<ILogger>();
    logger->Log("Application started", Severity::Info);

    // Main loop
    bool running = true;
    while (running) {
        plugify->Update();
    }

    plugify->Terminate();
    return 0;
}
*/