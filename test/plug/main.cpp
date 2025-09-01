#include "plugify/plugify.hpp"
#include "plugify/manager.hpp"
#include "plugify/plugify.hpp"
#include "plugify/extension.hpp"
#include "plugify/logger.hpp"

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

#include <CLI/CLI.hpp>
#include <glaze/glaze.hpp>
#include <plg/format.hpp>

#define PLG_COUT(x) std::cout << x << std::endl
#define PLG_CERR(x) std::cerr << x << std::endl
#define PLG_COUT_FMT(...) std::cout << std::format(__VA_ARGS__) << std::endl
#define PLG_CERR_FMT(...) std::cerr << std::format(__VA_ARGS__) << std::endl

using namespace plugify;

namespace {
    UniqueId FormatId(std::string_view str) {
        UniqueId::Value result;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);

        if (ec != std::errc{}) {
            PLG_CERR_FMT("Error: {}", std::make_error_code(ec).message());
            return {};
        } else if (ptr != str.data() + str.size()) {
            PLG_CERR("Invalid argument: trailing characters after the valid part");
            return {};
        }

        return UniqueId{result};
    }

    // ANSI Color codes
    struct Colors {
        static constexpr const std::string_view RESET = "\033[0m";
        static constexpr const std::string_view BOLD = "\033[1m";
        static constexpr const std::string_view RED = "\033[31m";
        static constexpr const std::string_view GREEN = "\033[32m";
        static constexpr const std::string_view YELLOW = "\033[33m";
        static constexpr const std::string_view BLUE = "\033[34m";
        static constexpr const std::string_view MAGENTA = "\033[35m";
        static constexpr const std::string_view CYAN = "\033[36m";
        static constexpr const std::string_view GRAY = "\033[90m";
        static constexpr const std::string_view BRIGHT_RED = "\033[91m";
        static constexpr const std::string_view BRIGHT_GREEN = "\033[92m";
        static constexpr const std::string_view BRIGHT_YELLOW = "\033[93m";
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
    std::string FormatDuration(Duration duration) {
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

    // Helper to get state color/symbol
    struct StateInfo {
        std::string_view symbol;
        std::string_view color;
    };

    StateInfo GetStateInfo(ExtensionState state) {
        switch (state) {
            case ExtensionState::Started:
            case ExtensionState::Loaded:
                return { "✓", Colors::GREEN };
            case ExtensionState::Failed:
            case ExtensionState::Corrupted:
                return { "✗", Colors::RED };
            case ExtensionState::Unresolved:
                return { "⚠", Colors::YELLOW };
            case ExtensionState::Disabled:
            case ExtensionState::Skipped:
                return { "○", Colors::GRAY };
            case ExtensionState::Loading:
            case ExtensionState::Starting:
            case ExtensionState::Parsing:
            case ExtensionState::Resolving:
                return { "⋯", Colors::CYAN };
            default:
                return { "?", Colors::GRAY };
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
        if (fs::is_regular_file(path)) {
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
            auto lang = ext->GetLanguage();
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
        j["id"] = UniqueId::Value{ext->GetId()};
        j["name"] = ext->GetName();
        j["version"] = ext->GetVersionString();
        j["type"] = ext->IsPlugin() ? "plugin" : "module";
        j["state"] = plg::enum_to_string(ext->GetState());
        j["language"] = ext->GetLanguage();
        j["location"] = ext->GetLocation().string();

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
            j["dependencies"] = glz::json_t::array_t();
            for (const auto& dep : ext->GetDependencies()) {
                glz::json_t::object_t depJson;
                depJson["name"] = dep.GetName();
                depJson["constraints"] = dep.GetConstraints().to_string();
                depJson["optional"] = dep.IsOptional();
                j["dependencies"].as<glz::json_t::array_t>().push_back(std::move(depJson));
            }
        }

        // Performance
        j["performance"]["total_time_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(ext->GetTotalTime()).count();

        // Errors and warnings
        if (ext->HasErrors()) {
            glz::json_t e = glz::json_t::array_t();
            for (const auto& error : ext->GetErrors()) {
                e.as<glz::json_t::array_t>().push_back(error);
            }
            j["errors"] = std::move(e);
        }
        if (ext->HasWarnings()) {
            glz::json_t w = glz::json_t::array_t();
            for (const auto& warning : ext->GetWarnings()) {
                w.as<glz::json_t::array_t>().push_back(warning);
            }
            j["warnings"] = w;
        }

        return j;
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

        PLG_COUT_FMT(
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
            auto depExt = manager.FindExtension(dep.GetName());
            if (depExt) {
                PrintDependencyTree(depExt, manager, newPrefix, lastDep);
            } else {
                std::string depConnector = lastDep ? "└─ " : "├─ ";
                std::string status = dep.IsOptional() ? "[optional]" : "[required]";
                PLG_COUT_FMT(
                    "{}{}○ {} {} {} {}",
                    newPrefix,
                    depConnector,
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
        plg::flat_map<std::string, size_t> statistics;
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

    PlugifyApp(std::shared_ptr<Plugify> plugify)
        : plug(std::move(plugify)) {
    }

    void Initialize() {
        if (!plug->Initialize()) {
            PLG_CERR("No feet, no sweets!");
            throw std::runtime_error("Failed to initialize Plugify");
        }
        plug->GetManager().Initialize();
        PLG_COUT("Plugin system initialized.");
    }

    void Terminate() {
        plug->Terminate();
        PLG_COUT("Plugin system terminated.");
    }

    void ShowVersion() {
        static std::string copyright = std::format(
            "Copyright (C) 2023-{}{}{}{} Untrusted Modders Team",
            __DATE__[7],
            __DATE__[8],
            __DATE__[9],
            __DATE__[10]
        );
        PLG_COUT(R"(      ____)");
        PLG_COUT(R"( ____|    \         Plugify )" << plug->GetVersion().to_string());
        PLG_COUT(R"((____|     `._____  )" << copyright);
        PLG_COUT(R"( ____|       _|___)");
        PLG_COUT(R"((____|     .'       This program may be freely redistributed under)");
        PLG_COUT(R"(     |____/         the terms of the MIT License.)");
    }

    void LoadManager() {
        if (!plug->IsInitialized()) {
            PLG_CERR("Initialize system before use.");
            return;
        }
        const auto& manager = plug->GetManager();
        if (manager.IsInitialized()) {
            PLG_CERR("Plugin manager already loaded.");
        } else {
            manager.Initialize();
            PLG_COUT("Plugin manager was loaded.");
        }
    }

    void UnloadManager() {
        if (!plug->IsInitialized()) {
            PLG_CERR("Initialize system before use.");
            return;
        }
        const auto& manager = plug->GetManager();
        if (!manager.IsInitialized()) {
            PLG_CERR("Plugin manager already unloaded.");
        } else {
            manager.Terminate();
            PLG_COUT("Plugin manager was unloaded.");
        }
    }

    void ReloadManager() {
        if (!plug->IsInitialized()) {
            PLG_CERR("Initialize system before use.");
            return;
        }
        const auto& manager = plug->GetManager();
        if (!manager.IsInitialized()) {
            PLG_CERR("Plugin manager not loaded.");
        } else {
            manager.Terminate();
            manager.Initialize();
            PLG_COUT("Plugin manager was reloaded.");
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
        std::vector<const Extension*> filtered;
        for (const auto& plugin : plugins) {
            if (MatchesFilter(plugin, filter)) {
                filtered.push_back(plugin);
            }
        }

        // Sort
        std::ranges::sort(filtered, [sortBy, reverseSort](const Extension* a, const Extension* b) {
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
            return reverseSort ? !result : result;
        });

        // Output
        if (jsonOutput) {
            glz::json_t j = glz::json_t::array_t();
            for (const auto& plugin : filtered) {
                j.as<glz::json_t::array_t>().push_back(ExtensionToJson(plugin));
            }
            PLG_COUT(*j.dump());
            return;
        }

        auto count = filtered.size();
        if (!count) {
            PLG_CERR(Colorize("No plugins found matching criteria.", Colors::YELLOW));
            return;
        }

        PLG_COUT_FMT(
            "{}:",
            Colorize(std::format("Listing {} plugin{}", count, (count > 1) ? "s" : ""), Colors::BOLD)
        );
        PLG_COUT(std::string(80, '-'));

        // Header
        PLG_COUT_FMT(
            "{} {} {} {} {} {}",
            Colorize(std::format("{:<3}", "#"), Colors::GRAY),
            Colorize(std::format("{:<25}", "Name"), Colors::GRAY),
            Colorize(std::format("{:<15}", "Version"), Colors::GRAY),
            Colorize(std::format("{:<12}", "State"), Colors::GRAY),
            Colorize(std::format("{:<8}", "Lang"), Colors::GRAY),
            Colorize(std::format("{:<12}", "Load Time"), Colors::GRAY)
        );
        PLG_COUT(std::string(80, '-'));

        size_t index = 1;
        for (const auto& plugin : filtered) {
            auto state = plugin->GetState();
            auto stateStr = plg::enum_to_string(state);
            auto [symbol, color] = GetStateInfo(state);

            // Get load time if available
            std::string loadTime = "N/A";
            try {
                auto duration = plugin->GetOperationTime(ExtensionState::Loaded);
                if (duration.count() > 0) {
                    loadTime = FormatDuration(duration);
                }
            } catch (...) {}

            PLG_COUT_FMT(
                "{:<3} {:<25} {:<15} {} {:<11} {:<8} {:<12}",
                index++,
                Truncate(plugin->GetName(), 24),
                plugin->GetVersionString(),
                Colorize(symbol, color),
                Truncate(std::string(stateStr), 10),
                Truncate(plugin->GetLanguage(), 7),
                loadTime
            );

            // Show errors/warnings if any
            if (plugin->HasErrors()) {
                for (const auto& error : plugin->GetErrors()) {
                    PLG_CERR_FMT("     └─ {}: {}", Colorize("Error", Colors::RED), error);
                }
            }
            if (plugin->HasWarnings()) {
                for (const auto& warning : plugin->GetWarnings()) {
                    PLG_COUT_FMT("     └─ {}: {}", Colorize("Warning", Colors::YELLOW), warning);
                }
            }
        }
        PLG_COUT(std::string(80, '-'));

        // Summary
        if (filter.states.has_value() || filter.languages.has_value()
            || filter.searchQuery.has_value() || filter.showOnlyFailed) {
            PLG_COUT_FMT(
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
        std::vector<const Extension*> filtered;
        for (const auto& module : modules) {
            if (MatchesFilter(module, filter)) {
                filtered.push_back(module);
            }
        }

        // Sort
        std::ranges::sort(filtered, [sortBy, reverseSort](const Extension* a, const Extension* b) {
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
            return reverseSort ? !result : result;
        });

        // Output
        if (jsonOutput) {
            glz::json_t j = glz::json_t::array_t();
            for (const auto& module : filtered) {
                j.as<glz::json_t::array_t>().push_back(ExtensionToJson(module));
            }
            PLG_COUT(*j.dump());
            return;
        }

        auto count = filtered.size();
        if (!count) {
            PLG_CERR(Colorize("No modules found matching criteria.", Colors::YELLOW));
            return;
        }

        PLG_COUT_FMT(
            "{}:",
            Colorize(std::format("Listing {} module{}", count, (count > 1) ? "s" : ""), Colors::BOLD)
        );
        PLG_COUT(std::string(80, '-'));

        // Header
        PLG_COUT_FMT(
            "{} {} {} {} {} {}",
            Colorize(std::format("{:<3}", "#"), Colors::GRAY),
            Colorize(std::format("{:<25}", "Name"), Colors::GRAY),
            Colorize(std::format("{:<15}", "Version"), Colors::GRAY),
            Colorize(std::format("{:<12}", "State"), Colors::GRAY),
            Colorize(std::format("{:<8}", "Lang"), Colors::GRAY),
            Colorize(std::format("{:<12}", "Load Time"), Colors::GRAY)
        );
        PLG_COUT(std::string(80, '-'));

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

            PLG_COUT_FMT(
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
                    PLG_CERR_FMT("     └─ {}: {}", Colorize("Error", Colors::RED), error);
                }
            }
            if (module->HasWarnings()) {
                for (const auto& warning : module->GetWarnings()) {
                    PLG_COUT_FMT("     └─ {}: {}", Colorize("Warning", Colors::YELLOW), warning);
                }
            }
        }
        PLG_COUT(std::string(80, '-'));

        // Summary
        if (filter.states.has_value() || filter.languages.has_value()
            || filter.searchQuery.has_value() || filter.showOnlyFailed) {
            PLG_COUT_FMT(
                "{}",
                Colorize(
                    std::format("Filtered: {} of {} total modules shown", filtered.size(), modules.size()),
                    Colors::GRAY
                )
            );
        }
    }

    void ShowPlugin(std::string_view name, bool useId = false, bool jsonOutput = false) {
        if (!CheckManager()) {
            return;
        }

        const auto& manager = plug->GetManager();
        auto plugin = useId ? manager.FindExtension(FormatId(name)) : manager.FindExtension(name);

        if (!plugin) {
            if (jsonOutput) {
                glz::json_t error;
                error["error"] = std::format("Plugin {} not found", name);
                PLG_COUT(*error.dump());
            } else {
                PLG_CERR_FMT("{}: Plugin {} not found.", Colorize("Error", Colors::RED), name);
            }
            return;
        }

        if (!plugin->IsPlugin()) {
            if (jsonOutput) {
                glz::json_t error;
                error["error"] = std::format("'{}' is not a plugin (it's a module)", name);
                PLG_COUT(*error.dump());
            } else {
                PLG_CERR_FMT(
                    "{}: '{}' is not a plugin (it's a module).",
                    Colorize("Error", Colors::RED),
                    name
                );
            }
            return;
        }

        // JSON output
        if (jsonOutput) {
            PLG_COUT(*ExtensionToJson(plugin).dump());
            return;
        }

        // Display detailed plugin information with colors
        PLG_COUT(std::string(80, '='));
        PLG_COUT_FMT(
            "{}: {}",
            Colorize("PLUGIN INFORMATION", Colors::BOLD),
            Colorize(plugin->GetName(), Colors::CYAN)
        );
        PLG_COUT(std::string(80, '='));

        // Status indicator
        auto [symbol, stateColor] = GetStateInfo(plugin->GetState());
        PLG_COUT_FMT(
            "\n{} {} {}",
            Colorize(symbol, stateColor),
            Colorize("Status:", Colors::BOLD),
            Colorize(plg::enum_to_string(plugin->GetState()), stateColor)
        );

        // Basic Information
        PLG_COUT(Colorize("\n[Basic Information]", Colors::CYAN));
        PLG_COUT_FMT("  {:<15} {}", Colorize("ID:", Colors::GRAY), plugin->GetId());
        PLG_COUT_FMT("  {:<15} {}", Colorize("Name:", Colors::GRAY), plugin->GetName());
        PLG_COUT_FMT(
            "  {:<15} {}",
            Colorize("Version:", Colors::GRAY),
            Colorize(plugin->GetVersionString(), Colors::GREEN)
        );
        PLG_COUT_FMT("  {:<15} {}", Colorize("Language:", Colors::GRAY), plugin->GetLanguage());
        PLG_COUT_FMT("  {:<15} {}", Colorize("Location:", Colors::GRAY), plugin->GetLocation().string());
        PLG_COUT_FMT(
            "  {:<15} {}",
            Colorize("File Size:", Colors::GRAY),
            FormatFileSize(plugin->GetLocation())
        );

        // Optional Information
        if (!plugin->GetDescription().empty() || !plugin->GetAuthor().empty()
            || !plugin->GetWebsite().empty() || !plugin->GetLicense().empty()) {
            PLG_COUT(Colorize("\n[Additional Information]", Colors::CYAN));
            if (!plugin->GetDescription().empty()) {
                PLG_COUT_FMT(
                    "  {:<15} {}",
                    Colorize("Description:", Colors::GRAY),
                    plugin->GetDescription()
                );
            }
            if (!plugin->GetAuthor().empty()) {
                PLG_COUT_FMT(
                    "  {:<15} {}",
                    Colorize("Author:", Colors::GRAY),
                    Colorize(plugin->GetAuthor(), Colors::MAGENTA)
                );
            }
            if (!plugin->GetWebsite().empty()) {
                PLG_COUT_FMT(
                    "  {:<15} {}",
                    Colorize("Website:", Colors::GRAY),
                    Colorize(plugin->GetWebsite(), Colors::BLUE)
                );
            }
            if (!plugin->GetLicense().empty()) {
                PLG_COUT_FMT("  {:<15} {}", Colorize("License:", Colors::GRAY), plugin->GetLicense());
            }
        }

        // Plugin-specific information
        if (!plugin->GetEntry().empty()) {
            PLG_COUT(Colorize("\n[Plugin Details]", Colors::CYAN));
            PLG_COUT_FMT(
                "  {:<15} {}",
                Colorize("Entry Point:", Colors::GRAY),
                Colorize(plugin->GetEntry(), Colors::YELLOW)
            );
        }

        // Methods
        const auto& methods = plugin->GetMethods();
        if (!methods.empty()) {
            PLG_COUT(
                Colorize("\n[Exported Methods]", Colors::CYAN)
                + Colorize(std::format(" ({} total)", methods.size()), Colors::GRAY)
            );

            size_t displayCount = std::min<size_t>(methods.size(), 10);
            for (size_t i = 0; i < displayCount; ++i) {
                const auto& method = methods[i];
                PLG_COUT_FMT(
                    "  {}{:<2} {}",
                    Colorize("#", Colors::GRAY),
                    i + 1,
                    Colorize(method.GetName(), Colors::GREEN)
                );
                if (!method.GetFuncName().empty()) {
                    PLG_COUT_FMT(
                        "      {} {}",
                        Colorize("Func Name:", Colors::GRAY),
                        method.GetFuncName()
                    );
                }
            }
            if (methods.size() > 10) {
                PLG_COUT_FMT(
                    "  {} ... and {} more methods",
                    Colorize("→", Colors::GRAY),
                    methods.size() - 10
                );
            }
        }

        // Platforms
        const auto& platforms = plugin->GetPlatforms();
        if (!platforms.empty()) {
            PLG_COUT(Colorize("\n[Supported Platforms]", Colors::CYAN));
            PLG_COUT_FMT("  {}", Colorize(plg::join(platforms, ", "), Colors::GREEN));
        }

        // Dependencies
        const auto& deps = plugin->GetDependencies();
        if (!deps.empty()) {
            PLG_COUT(
                Colorize("\n[Dependencies]", Colors::CYAN)
                + Colorize(std::format(" ({} total)", deps.size()), Colors::GRAY)
            );
            for (const auto& dep : deps) {
                std::string depIndicator = dep.IsOptional() ? Colorize("○", Colors::GRAY)
                                                            : Colorize("●", Colors::GREEN);
                PLG_COUT_FMT(
                    "  {} {} {}",
                    depIndicator,
                    Colorize(dep.GetName(), Colors::BOLD),
                    Colorize(dep.GetConstraints().to_string(), Colors::GRAY)
                );
                if (dep.IsOptional()) {
                    PLG_COUT_FMT("    └─ {}", Colorize("Optional", Colors::GRAY));
                }
            }
        }

        // Conflicts
        const auto& conflicts = plugin->GetConflicts();
        if (!conflicts.empty()) {
            PLG_COUT(
                Colorize("\n[Conflicts]", Colors::YELLOW)
                + Colorize(std::format(" ({} total)", conflicts.size()), Colors::GRAY)
            );
            for (const auto& conflict : conflicts) {
                PLG_COUT_FMT(
                    "  {} {} {}",
                    Colorize("⚠", Colors::YELLOW),
                    conflict.GetName(),
                    Colorize(conflict.GetConstraints().to_string(), Colors::GRAY)
                );
                if (!conflict.GetReason().empty()) {
                    PLG_COUT_FMT("    └─ {}", Colorize(conflict.GetReason(), Colors::RED));
                }
            }
        }

        // Performance Information
        PLG_COUT(Colorize("\n[Performance Metrics]", Colors::CYAN));
        auto totalTime = plugin->GetTotalTime();
        PLG_COUT_FMT(
            "  {:<15} {}",
            Colorize("Total Time:", Colors::GRAY),
            Colorize(
                FormatDuration(totalTime),
                totalTime > std::chrono::seconds(1) ? Colors::YELLOW : Colors::GREEN
            )
        );

        // Show timing for different operations
        ExtensionState operations[] = { ExtensionState::Parsing,
                                        ExtensionState::Resolving,
                                        ExtensionState::Loading,
                                        ExtensionState::Starting };

        for (const auto& op : operations) {
            try {
                auto duration = plugin->GetOperationTime(op);
                if (duration.count() > 0) {
                    bool slow = duration > std::chrono::milliseconds(500);
                    PLG_COUT_FMT(
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
            PLG_COUT(Colorize("\n[Issues]", Colors::RED));
            for (const auto& error : plugin->GetErrors()) {
                PLG_COUT_FMT("  {} {}", Colorize("ERROR:", Colors::RED), error);
            }
            for (const auto& warning : plugin->GetWarnings()) {
                PLG_COUT_FMT("  {} {}", Colorize("WARNING:", Colors::YELLOW), warning);
            }
        } else {
            PLG_COUT_FMT(
                "\n{} {}",
                Colorize("✓", Colors::GREEN),
                Colorize("No issues detected", Colors::GREEN)
            );
        }

        PLG_COUT(std::string(80, '='));
    }

    void ShowModule(std::string_view name, bool useId = false, bool jsonOutput = false) {
        if (!CheckManager()) {
            return;
        }

        const auto& manager = plug->GetManager();
        auto module = useId ? manager.FindExtension(FormatId(name)) : manager.FindExtension(name);

        if (!module) {
            if (jsonOutput) {
                glz::json_t error;
                error["error"] = std::format("Module {} not found", name);
                PLG_COUT(*error.dump());
            } else {
                PLG_CERR_FMT("{}: Module {} not found.", Colorize("Error", Colors::RED), name);
            }
            return;
        }

        if (!module->IsModule()) {
            if (jsonOutput) {
                glz::json_t error;
                error["error"] = std::format("'{}' is not a module (it's a plugin)", name);
                PLG_COUT(*error.dump());
            } else {
                PLG_CERR_FMT(
                    "{}: '{}' is not a module (it's a plugin).",
                    Colorize("Error", Colors::RED),
                    name
                );
            }
            return;
        }

        // JSON output
        if (jsonOutput) {
            PLG_COUT(*ExtensionToJson(module).dump());
            return;
        }

        // Display detailed module information with colors
        PLG_COUT(std::string(80, '='));
        PLG_COUT_FMT(
            "{}: {}",
            Colorize("MODULE INFORMATION", Colors::BOLD),
            Colorize(module->GetName(), Colors::MAGENTA)
        );
        PLG_COUT(std::string(80, '='));

        // Status indicator
        auto [symbol, stateColor] = GetStateInfo(module->GetState());
        PLG_COUT_FMT(
            "\n{} {} {}",
            Colorize(symbol, stateColor),
            Colorize("Status:", Colors::BOLD),
            Colorize(plg::enum_to_string(module->GetState()), stateColor)
        );

        // Basic Information
        PLG_COUT(Colorize("\n[Basic Information]", Colors::CYAN));
        PLG_COUT_FMT("  {:<15} {}", Colorize("ID:", Colors::GRAY), module->GetId());
        PLG_COUT_FMT("  {:<15} {}", Colorize("Name:", Colors::GRAY), module->GetName());
        PLG_COUT_FMT(
            "  {:<15} {}",
            Colorize("Version:", Colors::GRAY),
            Colorize(module->GetVersionString(), Colors::GREEN)
        );
        PLG_COUT_FMT("  {:<15} {}", Colorize("Language:", Colors::GRAY), module->GetLanguage());
        PLG_COUT_FMT("  {:<15} {}", Colorize("Location:", Colors::GRAY), module->GetLocation().string());
        PLG_COUT_FMT(
            "  {:<15} {}",
            Colorize("File Size:", Colors::GRAY),
            FormatFileSize(module->GetLocation())
        );

        // Optional Information
        if (!module->GetDescription().empty() || !module->GetAuthor().empty()
            || !module->GetWebsite().empty() || !module->GetLicense().empty()) {
            PLG_COUT(Colorize("\n[Additional Information]", Colors::CYAN));
            if (!module->GetDescription().empty()) {
                PLG_COUT_FMT(
                    "  {:<15} {}",
                    Colorize("Description:", Colors::GRAY),
                    module->GetDescription()
                );
            }
            if (!module->GetAuthor().empty()) {
                PLG_COUT_FMT(
                    "  {:<15} {}",
                    Colorize("Author:", Colors::GRAY),
                    Colorize(module->GetAuthor(), Colors::MAGENTA)
                );
            }
            if (!module->GetWebsite().empty()) {
                PLG_COUT_FMT(
                    "  {:<15} {}",
                    Colorize("Website:", Colors::GRAY),
                    Colorize(module->GetWebsite(), Colors::BLUE)
                );
            }
            if (!module->GetLicense().empty()) {
                PLG_COUT_FMT("  {:<15} {}", Colorize("License:", Colors::GRAY), module->GetLicense());
            }
        }

        // Module-specific information
        if (!module->GetRuntime().empty()) {
            PLG_COUT(Colorize("\n[Module Details]", Colors::CYAN));
            PLG_COUT_FMT(
                "  {:<15} {}",
                Colorize("Runtime:", Colors::GRAY),
                Colorize(module->GetRuntime().string(), Colors::YELLOW)
            );
        }

        // Directories
        const auto& dirs = module->GetDirectories();
        if (!dirs.empty()) {
            PLG_COUT(
                Colorize("\n[Search Directories]", Colors::CYAN)
                + Colorize(std::format(" ({} total)", dirs.size()), Colors::GRAY)
            );

            size_t displayCount = std::min<size_t>(dirs.size(), 5);
            for (size_t i = 0; i < displayCount; ++i) {
                bool exists = std::filesystem::exists(dirs[i]);
                PLG_COUT_FMT(
                    "  {} {}",
                    exists ? Colorize("✓", Colors::GREEN) : Colorize("✗", Colors::RED),
                    dirs[i].string()
                );
            }
            if (dirs.size() > 5) {
                PLG_COUT_FMT(
                    "  {} ... and {} more directories",
                    Colorize("→", Colors::GRAY),
                    dirs.size() - 5
                );
            }
        }

        // Assembly Information
        if (auto assembly = module->GetAssembly()) {
            PLG_COUT(Colorize("\n[Assembly Information]", Colors::CYAN));
            PLG_COUT_FMT("  {} Assembly loaded and active", Colorize("✓", Colors::GREEN));
            // Add more assembly details if available in your IAssembly interface
        }

        // Platforms
        const auto& platforms = module->GetPlatforms();
        if (!platforms.empty()) {
            PLG_COUT(Colorize("\n[Supported Platforms]", Colors::CYAN));
            PLG_COUT_FMT("  {}", Colorize(plg::join(platforms, ", "), Colors::GREEN));
        }

        // Dependencies
        const auto& deps = module->GetDependencies();
        if (!deps.empty()) {
            PLG_COUT(
                Colorize("\n[Dependencies]", Colors::CYAN)
                + Colorize(std::format(" ({} total)", deps.size()), Colors::GRAY)
            );
            for (const auto& dep : deps) {
                std::string depIndicator = dep.IsOptional() ? Colorize("○", Colors::GRAY)
                                                            : Colorize("●", Colors::GREEN);
                PLG_COUT_FMT(
                    "  {} {} {}",
                    depIndicator,
                    Colorize(dep.GetName(), Colors::BOLD),
                    Colorize(dep.GetConstraints().to_string(), Colors::GRAY)
                );
                if (dep.IsOptional()) {
                    PLG_COUT_FMT("    └─ {}", Colorize("Optional", Colors::GRAY));
                }
            }
        }

        // Conflicts
        const auto& conflicts = module->GetConflicts();
        if (!conflicts.empty()) {
            PLG_COUT(
                Colorize("\n[Conflicts]", Colors::YELLOW)
                + Colorize(std::format(" ({} total)", conflicts.size()), Colors::GRAY)
            );
            for (const auto& conflict : conflicts) {
                PLG_COUT_FMT(
                    "  {} {} {}",
                    Colorize("⚠", Colors::YELLOW),
                    conflict.GetName(),
                    Colorize(conflict.GetConstraints().to_string(), Colors::GRAY)
                );
                if (!conflict.GetReason().empty()) {
                    PLG_COUT_FMT("    └─ {}", Colorize(conflict.GetReason(), Colors::RED));
                }
            }
        }

        // Performance Information
        PLG_COUT(Colorize("\n[Performance Metrics]", Colors::CYAN));
        auto totalTime = module->GetTotalTime();
        PLG_COUT_FMT(
            "  {:<15} {}",
            Colorize("Total Time:", Colors::GRAY),
            Colorize(
                FormatDuration(totalTime),
                totalTime > std::chrono::seconds(1) ? Colors::YELLOW : Colors::GREEN
            )
        );

        // Show timing for different operations
        ExtensionState operations[] = { ExtensionState::Parsing,
                                        ExtensionState::Resolving,
                                        ExtensionState::Loading,
                                        ExtensionState::Starting };

        for (const auto& op : operations) {
            try {
                auto duration = module->GetOperationTime(op);
                if (duration.count() > 0) {
                    bool slow = duration > std::chrono::milliseconds(500);
                    PLG_COUT_FMT(
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
            PLG_COUT(Colorize("\n[Issues]", Colors::RED));
            for (const auto& error : module->GetErrors()) {
                PLG_COUT_FMT("  {} {}", Colorize("ERROR:", Colors::RED), error);
            }
            for (const auto& warning : module->GetWarnings()) {
                PLG_COUT_FMT("  {} {}", Colorize("WARNING:", Colors::YELLOW), warning);
            }
        } else {
            PLG_COUT_FMT(
                "\n{} {}",
                Colorize("✓", Colors::GREEN),
                Colorize("No issues detected", Colors::GREEN)
            );
        }

        PLG_COUT(std::string(80, '='));
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

        PLG_COUT(std::string(80, '='));
        PLG_COUT(Colorize("SYSTEM HEALTH CHECK", Colors::BOLD));
        PLG_COUT(std::string(80, '='));

        // Overall score
        PLG_COUT_FMT(
            "\n{}: {} {}",
            Colorize("Overall Health Score", Colors::BOLD),
            Colorize(std::format("{}/100", report.score), scoreColor),
            Colorize(std::format("[{}]", status), scoreColor)
        );

        // Statistics
        PLG_COUT(Colorize("\n[Statistics]", Colors::CYAN));
        PLG_COUT_FMT("  Total Extensions:        {}", report.statistics["total_extensions"]);
        PLG_COUT_FMT(
            "  Failed Extensions:       {} {}",
            report.statistics["failed_extensions"],
            report.statistics["failed_extensions"] > 0 ? Colorize("⚠", Colors::RED)
                                                       : Colorize("✓", Colors::GREEN)
        );
        PLG_COUT_FMT(
            "  Extensions with Errors:  {} {}",
            report.statistics["extensions_with_errors"],
            report.statistics["extensions_with_errors"] > 0 ? Colorize("⚠", Colors::YELLOW)
                                                            : Colorize("✓", Colors::GREEN)
        );
        PLG_COUT_FMT("  Total Warnings:          {}", report.statistics["total_warnings"]);
        PLG_COUT_FMT("  Slow Loading Extensions: {}", report.statistics["slow_loading_extensions"]);

        // Issues
        if (!report.issues.empty()) {
            PLG_COUT(Colorize("\n[Critical Issues]", Colors::RED));
            for (const auto& issue : report.issues) {
                PLG_COUT_FMT("  {} {}", Colorize("✗", Colors::RED), issue);
            }
        }

        // Warnings
        if (!report.warnings.empty()) {
            PLG_COUT(Colorize("\n[Warnings]", Colors::YELLOW));
            for (const auto& warning : report.warnings) {
                PLG_COUT_FMT("  {} {}", Colorize("⚠", Colors::YELLOW), warning);
            }
        }

        // Recommendations
        PLG_COUT(Colorize("\n[Recommendations]", Colors::CYAN));
        if (report.score == 100) {
            PLG_COUT_FMT("  {} System is running optimally!", Colorize("✓", Colors::GREEN));
        } else {
            if (report.statistics["failed_extensions"] > 0) {
                PLG_COUT("  • Fix or remove failed extensions");
            }
            if (report.statistics["extensions_with_errors"] > 0) {
                PLG_COUT("  • Review and resolve extension errors");
            }
            if (report.statistics["slow_loading_extensions"] > 0) {
                PLG_COUT("  • Investigate slow-loading extensions for optimization");
            }
        }

        PLG_COUT(std::string(80, '='));
    }

    void ShowDependencyTree(std::string_view name, bool useId = false) {
        if (!CheckManager()) {
            return;
        }

        const auto& manager = plug->GetManager();
        auto ext = useId ? manager.FindExtension(FormatId(name)) : manager.FindExtension(name);

        if (!ext) {
            PLG_CERR_FMT("{} {} not found.", Colorize("Error:", Colors::RED), name);
            return;
        }

        PLG_COUT(std::string(80, '='));
        PLG_COUT_FMT("{}: {}", Colorize("DEPENDENCY TREE", Colors::BOLD), ext->GetName());
        PLG_COUT(std::string(80, '='));
        PLG_COUT("");

        PrintDependencyTree(ext, manager);

        // Also show what depends on this extension
        PLG_COUT(Colorize("\n[Reverse Dependencies]", Colors::CYAN));
        PLG_COUT("Extensions that depend on this:");

        bool found = false;
        for (const auto& other : manager.GetExtensions()) {
            for (const auto& dep : other->GetDependencies()) {
                if (dep.GetName() == ext->GetName()) {
                    PLG_COUT_FMT(
                        "  • {} {}",
                        other->GetName(),
                        dep.IsOptional() ? Colorize("[optional]", Colors::GRAY) : ""
                    );
                    found = true;
                }
            }
        }

        if (!found) {
            PLG_COUT_FMT("  {}", Colorize("None", Colors::GRAY));
        }

        PLG_COUT(std::string(80, '='));
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
            PLG_COUT_FMT("{} No extensions found matching '{}'", Colorize("ℹ", Colors::YELLOW), query);
            return;
        }

        PLG_COUT_FMT(
            "{}: Found {} match{} for '{}'",
            Colorize("SEARCH RESULTS", Colors::BOLD),
            matches.size(),
            matches.size() > 1 ? "es" : "",
            query
        );
        PLG_COUT(std::string(80, '-'));

        for (const auto& ext : matches) {
            auto [symbol, color] = GetStateInfo(ext->GetState());
            PLG_COUT_FMT(
                "{} {} {} {} {}",
                Colorize(symbol, color),
                Colorize(ext->GetName(), Colors::BOLD),
                Colorize(ext->GetVersionString(), Colors::GRAY),
                ext->IsPlugin() ? "[Plugin]" : "[Module]",
                Colorize(std::format("({})", ext->GetLanguage()), Colors::GRAY)
            );

            if (!ext->GetDescription().empty()) {
                PLG_COUT_FMT("  {}", Truncate(ext->GetDescription(), 70));
            }
        }
        PLG_COUT(std::string(80, '-'));
    }

    void ValidateExtension(const std::filesystem::path& path) {
        PLG_COUT_FMT("{}: {}", Colorize("VALIDATING", Colors::BOLD), path.string());
        PLG_COUT(std::string(80, '-'));

        // Check if file exists
        if (!std::filesystem::exists(path)) {
            PLG_CERR_FMT("{} File does not exist", Colorize("✗", Colors::RED));
            return;
        }

        // Check file extension
        auto fileExt = path.extension().string();
        bool isPlugin = (fileExt == ".plg" || fileExt == ".pplugin");
        bool isModule = (fileExt == ".mod" || fileExt == ".pmodule");

        if (!isPlugin && !isModule) {
            PLG_CERR_FMT("{} Invalid file extension: {}", Colorize("✗", Colors::RED), fileExt);
            return;
        }

        PLG_COUT_FMT("{} File exists", Colorize("✓", Colors::GREEN));
        PLG_COUT_FMT(
            "{} Valid extension type: {}",
            Colorize("✓", Colors::GREEN),
            isPlugin ? "Plugin" : "Module"
        );
        PLG_COUT_FMT("{} File size: {}", Colorize("ℹ", Colors::CYAN), FormatFileSize(path));

        // Try to parse manifest (you'd need to implement manifest parsing)
        // This is a placeholder for actual validation logic
        /*PLG_COUT_FMT("{} Manifest validation: {}",
                    Colorize("ℹ", Colors::CYAN),
                    "Would parse and validate manifest here");*/
        // TODO

        PLG_COUT(std::string(80, '-'));
        PLG_COUT_FMT("{}: Validation complete", Colorize("RESULT", Colors::BOLD));
    }

    void CompareExtensions(std::string_view name1, std::string_view name2, bool useId = false) {
        if (!CheckManager()) {
            return;
        }

        const auto& manager = plug->GetManager();
        auto ext1 = useId ? manager.FindExtension(FormatId(name1)) : manager.FindExtension(name1);
        auto ext2 = useId ? manager.FindExtension(FormatId(name2)) : manager.FindExtension(name2);

        if (!ext1) {
            PLG_CERR_FMT("Extension {} not found.", name1);
            return;
        }
        if (!ext2) {
            PLG_CERR_FMT("Extension {} not found.", name2);
            return;
        }

        PLG_COUT(std::string(80, '='));
        PLG_COUT(Colorize("EXTENSION COMPARISON", Colors::BOLD));
        PLG_COUT(std::string(80, '='));

        // Basic comparison table
        auto printRow = [](std::string_view label, std::string_view val1, std::string_view val2) {
            bool same = (val1 == val2);
            PLG_COUT_FMT("{:<20} {:<25} {} {:<25}", label, val1, same ? "=" : "≠", val2);
        };

        PLG_COUT_FMT(
            "\n{:<20} {:<25}   {:<25}",
            "",
            Colorize(ext1->GetName(), Colors::CYAN),
            Colorize(ext2->GetName(), Colors::MAGENTA)
        );
        PLG_COUT(std::string(80, '-'));

        printRow("Type:", ext1->IsPlugin() ? "Plugin" : "Module", ext2->IsPlugin() ? "Plugin" : "Module");
        printRow("Version:", ext1->GetVersionString(), ext2->GetVersionString());
        printRow("Language:", ext1->GetLanguage(), ext2->GetLanguage());
        printRow("State:", plg::enum_to_string(ext1->GetState()), plg::enum_to_string(ext2->GetState()));
        printRow("Author:", ext1->GetAuthor(), ext2->GetAuthor());
        printRow("License:", ext1->GetLicense(), ext2->GetLicense());

        // Dependencies comparison
        PLG_COUT(Colorize("\n[Dependencies]", Colors::BOLD));
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
            PLG_COUT_FMT("  Common: {}", plg::join(common, ", "));
        }
        if (!onlyIn1.empty()) {
            PLG_COUT_FMT("  Only in {}: {}", ext1->GetName(), plg::join(onlyIn1, ", "));
        }
        if (!onlyIn2.empty()) {
            PLG_COUT_FMT("  Only in {}: {}", ext2->GetName(), plg::join(onlyIn2, ", "));
        }

        // Performance comparison
        PLG_COUT(Colorize("\n[Performance]", Colors::BOLD));
        PLG_COUT_FMT(
            "  Load Time:     {:<15} vs {:<15}",
            FormatDuration(ext1->GetOperationTime(ExtensionState::Loaded)),
            FormatDuration(ext2->GetOperationTime(ExtensionState::Loaded))
        );
        PLG_COUT_FMT(
            "  Total Time:    {:<15} vs {:<15}",
            FormatDuration(ext1->GetTotalTime()),
            FormatDuration(ext2->GetTotalTime())
        );

        PLG_COUT(std::string(80, '='));
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
            PLG_CERR("Initialize system before use.");
            return false;
        }
        const auto& manager = plug->GetManager();
        if (!manager.IsInitialized()) {
            PLG_CERR("You must load plugin manager before query any information from it.");
            return false;
        }
        return true;
    }

    std::shared_ptr<Plugify> plug;
};

// Enhanced interactive mode
void
RunEnhancedInteractiveMode(PlugifyApp& app) {
    PLG_COUT(
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
            PLG_COUT(Colorize("Goodbye!", Colors::CYAN));
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
        interactiveApp.allow_extras();
        interactiveApp.prefix_command();

        // Add all commands (similar to main but simplified)
        auto* init = interactiveApp.add_subcommand("init", "Initialize");
        auto* term = interactiveApp.add_subcommand("term", "Terminate");
        auto* version = interactiveApp.add_subcommand("version", "Show version");
        auto* load = interactiveApp.add_subcommand("load", "Load manager");
        auto* unload = interactiveApp.add_subcommand("unload", "Unload manager");
        auto* reload = interactiveApp.add_subcommand("reload", "Reload manager");
        auto* plugins = interactiveApp.add_subcommand("plugins", "List plugins");
        auto* modules = interactiveApp.add_subcommand("modules", "List modules");
        auto* health = interactiveApp.add_subcommand("health", "System health");
        auto* search_cmd = interactiveApp.add_subcommand("search", "Search extensions");
        auto* help = interactiveApp.add_subcommand("help", "Show help");

        // Add options for plugins/modules
        bool showFailed = false;
        std::string sortBy = "name";
        plugins->add_flag("--failed", showFailed);
        plugins->add_option("--sort", sortBy);
        modules->add_flag("--failed", showFailed);
        modules->add_option("--sort", sortBy);

        std::string searchQuery;
        search_cmd->add_option("query", searchQuery);

        // Set callbacks
        init->callback([&app]() { app.Initialize(); });
        term->callback([&app]() { app.Terminate(); });
        version->callback([&app]() { app.ShowVersion(); });
        load->callback([&app]() { app.LoadManager(); });
        unload->callback([&app]() { app.UnloadManager(); });
        reload->callback([&app]() { app.ReloadManager(); });

        plugins->callback([&app, &showFailed, &sortBy]() {
            FilterOptions filter;
            filter.showOnlyFailed = showFailed;
            auto sort = sortBy == "name"      ? SortBy::Name
                        : sortBy == "state"   ? SortBy::State
                        : sortBy == "version" ? SortBy::Version
                                              : SortBy::Name;
            app.ListPlugins(filter, sort);
        });

        modules->callback([&app, &showFailed, &sortBy]() {
            FilterOptions filter;
            filter.showOnlyFailed = showFailed;
            auto sort = sortBy == "name"      ? SortBy::Name
                        : sortBy == "state"   ? SortBy::State
                        : sortBy == "version" ? SortBy::Version
                                              : SortBy::Name;
            app.ListModules(filter, sort);
        });

        health->callback([&app]() { app.ShowHealth(); });

        search_cmd->callback([&app, &searchQuery]() {
            if (!searchQuery.empty()) {
                app.SearchExtensions(searchQuery);
            } else {
                PLG_CERR("Search query required");
            }
        });

        help->callback([]() {
            PLG_COUT(Colorize("Plugify Interactive Mode Commands:", Colors::BOLD));
            PLG_COUT("  init                - Initialize plugin system");
            PLG_COUT("  term                - Terminate plugin system");
            PLG_COUT("  version             - Show version information");
            PLG_COUT("  load                - Load plugin manager");
            PLG_COUT("  unload              - Unload plugin manager");
            PLG_COUT("  reload              - Reload plugin manager");
            PLG_COUT("  plugins [--failed]  - List plugins (optionally only failed)");
            PLG_COUT("  modules [--failed]  - List modules (optionally only failed)");
            PLG_COUT("  health              - Show system health report");
            PLG_COUT("  search <query>      - Search extensions");
            PLG_COUT("  clear/cls           - Clear screen");
            PLG_COUT("  help                - Show this help");
            PLG_COUT("  exit/quit/q         - Exit interactive mode");
            PLG_COUT("");
            PLG_COUT("Advanced commands available in non-interactive mode:");
            PLG_COUT("  tree <name>         - Show dependency tree");
            PLG_COUT("  validate <path>     - Validate extension file");
            PLG_COUT("  compare <e1> <e2>   - Compare two extensions");
        });

        try {
            interactiveApp.parse(args);
        } catch (const CLI::ParseError& e) {
            if (e.get_exit_code() != 0) {
                PLG_CERR_FMT("{}: {}", Colorize("Error", Colors::RED), e.what());
                PLG_CERR("Type 'help' for available commands.");
            }
        }

        app.Update();
    }
}

// Updated main.cpp with enhanced CLI features

int
main(int argc, char** argv) {
    auto plugify = MakePlugify();
    if (!plugify) {
        PLG_COUT(plugify.error());
        return EXIT_FAILURE;
    }

    PlugifyApp app(std::move(*plugify));

    CLI::App cliApp{ "Plugify - Plugin Management System" };
    cliApp.set_version_flag("-v,--version", [&app]() {
        app.ShowVersion();
        return "";
    });

    // Global options
    bool interactive = false;
    bool noColor = false;
    bool jsonOutput = false;

    cliApp.add_flag("-i,--interactive", interactive, "Run in interactive mode");
    cliApp.add_flag("--no-color", noColor, "Disable colored output");
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
        ->add_option("--sort", pluginSortBy, "Sort by: name, version, state, language, loadtime")
        ->check(CLI::IsMember({ "name", "version", "state", "language", "loadtime" }));
    plugins_cmd->add_flag("--reverse,-r", pluginReverse, "Reverse sort order");
    plugins_cmd->add_flag("--failed", pluginShowFailed, "Show only failed plugins");

    auto* modules_cmd = cliApp.add_subcommand("modules", "List all modules");
    std::string moduleFilterState;
    std::string moduleFilterLang;
    std::string moduleSortBy = "name";
    bool moduleReverse = false;
    bool moduleShowFailed = false;

    modules_cmd->add_option("--filter-state", moduleFilterState, "Filter by state (comma-separated)");
    modules_cmd->add_option("--filter-lang", moduleFilterLang, "Filter by language (comma-separated)");
    modules_cmd
        ->add_option("--sort", moduleSortBy, "Sort by: name, version, state, language, loadtime")
        ->check(CLI::IsMember({ "name", "version", "state", "language", "loadtime" }));
    modules_cmd->add_flag("--reverse,-r", moduleReverse, "Reverse sort order");
    modules_cmd->add_flag("--failed", moduleShowFailed, "Show only failed modules");

    // Show plugin/module commands
    auto* plugin_cmd = cliApp.add_subcommand("plugin", "Show plugin information");
    std::string plugin_name;
    bool plugin_use_id = false;
    plugin_cmd->add_option("name", plugin_name, "Plugin name or ID")->required();
    plugin_cmd->add_flag("-u,--uuid", plugin_use_id, "Use ID instead of name");

    auto* module_cmd = cliApp.add_subcommand("module", "Show module information");
    std::string module_name;
    bool module_use_id = false;
    module_cmd->add_option("name", module_name, "Module name or ID")->required();
    module_cmd->add_flag("-u,--uuid", module_use_id, "Use ID instead of name");

    // New commands
    auto* health_cmd = cliApp.add_subcommand("health", "Show system health report");

    auto* tree_cmd = cliApp.add_subcommand("tree", "Show dependency tree");
    std::string tree_name;
    bool tree_use_id = false;
    tree_cmd->add_option("name", tree_name, "Extension name or ID")->required();
    tree_cmd->add_flag("-u,--uuid", tree_use_id, "Use ID instead of name");

    auto* search_cmd = cliApp.add_subcommand("search", "Search extensions");
    std::string search_query;
    search_cmd->add_option("query", search_query, "Search query")->required();

    auto* validate_cmd = cliApp.add_subcommand("validate", "Validate extension file");
    std::string validate_path;
    validate_cmd->add_option("path", validate_path, "Path to extension file")->required();

    auto* compare_cmd = cliApp.add_subcommand("compare", "Compare two extensions");
    std::string compare_ext1, compare_ext2;
    bool compare_use_id = false;
    compare_cmd->add_option("extension1", compare_ext1, "First extension")->required();
    compare_cmd->add_option("extension2", compare_ext2, "Second extension")->required();
    compare_cmd->add_flag("-u,--uuid", compare_use_id, "Use ID instead of name");

    // Helper function to parse comma-separated values
    auto parseCsv = [](const std::string& str) -> std::vector<std::string> {
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
    auto parseSortBy = [](std::string_view str) -> SortBy {
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
    auto parseStates = [](const std::vector<std::string>& strs) -> std::vector<ExtensionState> {
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
            filter.states = parseStates(parseCsv(pluginFilterState));
        }
        if (!pluginFilterLang.empty()) {
            filter.languages = parseCsv(pluginFilterLang);
        }
        filter.showOnlyFailed = pluginShowFailed;

        app.ListPlugins(filter, parseSortBy(pluginSortBy), pluginReverse, jsonOutput);
    });

    modules_cmd->callback([&]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }

        FilterOptions filter;
        if (!moduleFilterState.empty()) {
            filter.states = parseStates(parseCsv(moduleFilterState));
        }
        if (!moduleFilterLang.empty()) {
            filter.languages = parseCsv(moduleFilterLang);
        }
        filter.showOnlyFailed = moduleShowFailed;

        app.ListModules(filter, parseSortBy(moduleSortBy), moduleReverse, jsonOutput);
    });

    plugin_cmd->callback([&app, &plugin_name, &plugin_use_id, &jsonOutput]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }
        if (jsonOutput) {
            const auto& manager = app.GetManager();
            auto plugin = plugin_use_id ? manager.FindExtension(FormatId(plugin_name))
                                        : manager.FindExtension(plugin_name);
            if (plugin) {
                PLG_COUT(*ExtensionToJson(plugin).dump());
            } else {
                glz::json_t error;
                error["error"] = "Plugin not found";
                PLG_COUT(*error.dump());
            }
        } else {
            app.ShowPlugin(plugin_name, plugin_use_id);
        }
    });

    module_cmd->callback([&app, &module_name, &module_use_id, &jsonOutput]() {
        if (!app.IsInitialized()) {
            app.Initialize();
        }
        if (jsonOutput) {
            const auto& manager = app.GetManager();
            auto module = module_use_id ? manager.FindExtension(FormatId(module_name))
                                        : manager.FindExtension(module_name);
            if (module) {
                PLG_COUT(*ExtensionToJson(module).dump());
            } else {
                glz::json_t error;
                error["error"] = "Module not found";
                PLG_COUT(*error.dump());
            }
        } else {
            app.ShowModule(module_name, module_use_id);
        }
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
        PLG_CERR_FMT("{}: {}", Colorize("Error", Colors::RED), e.what());
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
    plugify::PluginManagerConfig config;
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

    // Access provider for convenient operations
    auto provider = plugify->GetProvider();
    provider->LogInfo("Application started");

    // Main loop
    bool running = true;
    while (running) {
        plugify->Update();
    }

    plugify->Terminate();
    return 0;
}
*/