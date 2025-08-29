#pragma once

#include <any>
#include <filesystem>
#include <optional>
#include <string>

#include "plugify/core/logger.hpp"
#include "plugify/core/types.hpp"

namespace plugify {
    struct Config {
        // Paths configuration
        struct Paths {
            std::filesystem::path baseDir;
            std::filesystem::path extensionsDir = "extensions";
            std::filesystem::path configsDir = "configs";
            std::filesystem::path dataDir = "data";
            std::filesystem::path logsDir = "logs";
            std::filesystem::path cacheDir = "cache";
        } paths;

        // Loading configuration
        struct Loading {
            bool preferOwnSymbols = true;
            //bool enableHotReload = false;
            //bool lazyLoading = false;
            //bool parallelLoading = true;
            size_t maxConcurrentLoads = 4;
            std::chrono::seconds loadTimeout{30};
        } loading;

        // Runtime configuration
        struct Runtime {
            //bool enableSandboxing = false;
            //bool enableProfiling = false;
            //size_t maxMemoryPerPlugin = 0; // 0 = unlimited
            std::chrono::milliseconds updateInterval{16}; // ~60 FPS
        } runtime;

        // Security configuration
        struct Security {
            //bool verifySignatures = false;
            //bool allowUnsignedPlugins = true;
            //std::vector<std::string> trustedPublishers;
            std::vector<std::string> whitelistedExtensions;
            std::vector<std::string> blacklistedExtensions;
        } security;

        // Logging configuration
        struct Logging {
            bool printReport = false;
            bool printLoadOrder = false;
            bool printDependencyGraph = false;
            std::filesystem::path exportDigraphDot;
            Severity severity{Severity::Error};
        } logging;

        // Retry configuration
        /*struct RetryPolicy {
            std::size_t maxAttempts = 3;
            std::chrono::milliseconds baseDelay{100};
            std::chrono::milliseconds maxDelay{5000};
            bool exponentialBackoff = true;
            bool retryOnlyTransient = true;
        } retryPolicy;*/
    };

	/*struct Config {
		// Initialization behavior
		bool partialStartupMode = true;
		bool failOnMissingDependencies = false;

		//bool failOnModuleError = false;
		//bool continueOnValidationWarnings = true;
		bool respectDependencyOrder = true;  // Initialize in dependency order
		bool skipDependentsOnFailure = true; // Skip extensions if their dependencies fail
		bool printSummary = true;  // Print initialization summary to console

		// Update behavior
		//bool failOnUpdateError = false;      // Whether to fail if any extension update fails
		//bool verboseUpdates = false;         // Log update errors to console
		//bool trackUpdatePerformance = true;  // Track update timing statistics
		//std::chrono::microseconds slowUpdateThreshold{16667}; // ~60 FPS threshold

		// Timeouts
		//std::chrono::milliseconds initializationTimeout{30000};
		//std::chrono::milliseconds perExtensionTimeout{5000};
	};*/
#if 0
    // TODO: Consider config system
    // Configuration value types
    using ConfigValue = std::variant<
        std::monostate,  // null
        bool,
        int64_t,
        double,
        std::string,
        std::vector<std::any>,  // array
        std::unordered_map<std::string, std::any>  // object
    >;

    // Configuration source priority (higher value = higher priority)
    enum class ConfigPriority {
        Default = 0,      // Built-in defaults
        File = 1,         // From config file
        Environment = 2,  // Environment variables
        CommandLine = 3,  // Command line args
        Runtime = 4       // Set at runtime
    };

    // Configuration node for hierarchical config
    class ConfigNode {
    public:
        ConfigNode() = default;
        explicit ConfigNode(ConfigValue value) : _value(std::move(value)) {}

        // Type-safe getters
        template<typename T>
        std::optional<T> Get() const {
            if (auto* val = std::get_if<T>(&_value)) {
                return *val;
            }
            return std::nullopt;
        }

        // Path-based access (e.g., "server.host.port")
        std::optional<ConfigNode> GetNode(std::string_view path) const;
        void SetNode(std::string_view path, ConfigNode node);

        // Merge another node into this one
        void Merge(const ConfigNode& other, ConfigPriority priority);

        // Check if node exists
        bool Has(std::string_view path) const;

        // Get all keys at current level
        std::vector<std::string> GetKeys() const;

    private:
        ConfigValue _value;
        ConfigPriority _priority = ConfigPriority::Default;
        std::unordered_map<std::string, ConfigNode> _children;
    };

    // Configuration schema for validation
    class ConfigSchema {
    public:
        struct Field {
            std::string name;
            std::string type;  // "bool", "int", "double", "string", "array", "object"
            bool required = false;
            std::optional<ConfigValue> defaultValue;
            std::optional<std::string> description;
            std::optional<std::function<bool(const ConfigValue&)>> validator;
            std::optional<std::vector<ConfigValue>> allowedValues;  // enum constraint
            std::optional<std::pair<ConfigValue, ConfigValue>> range;  // min/max
        };

        ConfigSchema& AddField(Field field);
        ConfigSchema& AddSubSchema(const std::string& path, const ConfigSchema& schema);

        Result<void> Validate(const ConfigNode& config) const;
        ConfigNode ApplyDefaults(const ConfigNode& config) const;
        std::string GenerateDocumentation() const;

    private:
        std::vector<Field> _fields;
        std::unordered_map<std::string, ConfigSchema> _subSchemas;
    };

    // Base config reader interface
    class IConfigReader {
    public:
        virtual ~IConfigReader() = default;
        virtual Result<ConfigNode> Read(const std::filesystem::path& path) = 0;
        virtual bool CanRead(const std::filesystem::path& path) const = 0;
    };

    // JSON config reader
    class JsonConfigReader : public IConfigReader {
    public:
        Result<ConfigNode> Read(const std::filesystem::path& path) override;
        bool CanRead(const std::filesystem::path& path) const override {
            auto ext = path.extension().string();
            return ext == ".json" || ext == ".jsonc";
        }
    };

    // YAML config reader
    class YamlConfigReader : public IConfigReader {
    public:
        Result<ConfigNode> Read(const std::filesystem::path& path) override;
        bool CanRead(const std::filesystem::path& path) const override {
            auto ext = path.extension().string();
            return ext == ".yaml" || ext == ".yml";
        }
    };

    // TOML config reader
    class TomlConfigReader : public IConfigReader {
    public:
        Result<ConfigNode> Read(const std::filesystem::path& path) override;
        bool CanRead(const std::filesystem::path& path) const override {
            return path.extension() == ".toml";
        }
    };

    // INI config reader
    class IniConfigReader : public IConfigReader {
    public:
        Result<ConfigNode> Read(const std::filesystem::path& path) override;
        bool CanRead(const std::filesystem::path& path) const override {
            auto ext = path.extension().string();
            return ext == ".ini" || ext == ".cfg";
        }
    };

    // Environment variable config source
    class EnvironmentConfigSource {
    public:
        // Configure prefix for env vars (e.g., "PLUGIFY_")
        explicit EnvironmentConfigSource(std::string prefix = "PLUGIFY_")
            : _prefix(std::move(prefix)) {}

        // Read all env vars with prefix and convert to config
        ConfigNode Read() const;

        // Map specific env vars to config paths
        EnvironmentConfigSource& Map(std::string_view envVar, std::string_view configPath);

    private:
        std::string _prefix;
        std::unordered_map<std::string, std::string> _mappings;
    };

    // Command line config source
    class CommandLineConfigSource {
    public:
        CommandLineConfigSource(int argc, char* argv[]);

        // Parse command line args to config
        Result<ConfigNode> Parse();

        // Define expected arguments
        CommandLineConfigSource& AddOption(
            std::string_view longName,
            std::optional<char> shortName,
            std::string_view configPath,
            std::string_view description);

        // Generate help text
        std::string GetHelp() const;

    private:
        std::vector<std::string> _args;
        struct Option {
            std::string longName;
            std::optional<char> shortName;
            std::string configPath;
            std::string description;
            bool requiresValue = true;
        };
        std::vector<Option> _options;
    };

    // Main configuration manager
    class ConfigManager {
    public:
        ConfigManager();

        // Register config readers
        void RegisterReader(std::unique_ptr<IConfigReader> reader);

        // Set schema for validation
        void SetSchema(ConfigSchema schema);

        // Load configuration from various sources
        Result<void> LoadFromFile(const std::filesystem::path& path,
                                 ConfigPriority priority = ConfigPriority::File);
        Result<void> LoadFromEnvironment(const EnvironmentConfigSource& source);
        Result<void> LoadFromCommandLine(int argc, char* argv[]);
        Result<void> LoadFromString(std::string_view content, std::string_view format);

        // Multiple config files with override behavior
        Result<void> LoadConfigFiles(const std::vector<std::filesystem::path>& paths);

        // Watch config file for changes
        Result<void> WatchFile(const std::filesystem::path& path,
                              std::function<void(const ConfigNode&)> onChange);
        void StopWatching(const std::filesystem::path& path);

        // Access configuration
        template<typename T>
        std::optional<T> Get(std::string_view path) const {
            if (auto node = _config.GetNode(path)) {
                return node->Get<T>();
            }
            return std::nullopt;
        }

        // Set configuration value
        template<typename T>
        void Set(std::string_view path, T value, ConfigPriority priority = ConfigPriority::Runtime) {
            ConfigNode node(ConfigValue(std::move(value)));
            _config.SetNode(path, std::move(node));
        }

        // Get with default
        template<typename T>
        T GetOr(std::string_view path, T defaultValue) const {
            return Get<T>(path).value_or(std::move(defaultValue));
        }

        // Get required value (throws if not found)
        template<typename T>
        T GetRequired(std::string_view path) const {
            auto value = Get<T>(path);
            if (!value) {
                throw std::runtime_error(
                    std::format("Required config value not found: {}", path));
            }
            return *value;
        }

        // Check if path exists
        bool Has(std::string_view path) const {
            return _config.Has(path);
        }

        // Export current config
        Result<void> SaveToFile(const std::filesystem::path& path) const;
        std::string ToString(std::string_view format = "json") const;

        // Validate current config against schema
        Result<void> Validate() const;

        // Get interpolated value (resolve ${var} references)
        template<typename T>
        std::optional<T> GetInterpolated(std::string_view path) const {
            auto value = Get<std::string>(path);
            if (!value) return std::nullopt;

            auto interpolated = InterpolateString(*value);
            if constexpr (std::is_same_v<T, std::string>) {
                return interpolated;
            } else {
                // Try to convert string to requested type
                return ConvertString<T>(interpolated);
            }
        }

        // Reload all config files
        Result<void> Reload();

        // Get config metadata
        struct Metadata {
            std::vector<std::filesystem::path> loadedFiles;
            std::chrono::system_clock::time_point lastReload;
            bool hasSchema;
            size_t configNodeCount;
        };
        Metadata GetMetadata() const { return _metadata; }

    private:
        ConfigNode _config;
        ConfigSchema _schema;
        std::vector<std::unique_ptr<IConfigReader>> _readers;
        std::vector<std::filesystem::path> _loadedFiles;
        Metadata _metadata;

        std::string InterpolateString(std::string_view str) const;

        template<typename T>
        std::optional<T> ConvertString(const std::string& str) const;

        // File watching implementation
        //struct FileWatcher;
       // std::unique_ptr<FileWatcher> _fileWatcher;
    };

    // Specialized config for Plugify
    struct PlugifyConfig {
        // Core settings
        std::filesystem::path baseDir = std::filesystem::current_path();
        std::filesystem::path pluginsDir = baseDir / "plugins";
        std::filesystem::path configsDir = baseDir / "configs";
        std::filesystem::path dataDir = baseDir / "data";
        std::filesystem::path logsDir = baseDir / "logs";
        std::filesystem::path cacheDir = baseDir / "cache";

        // Feature flags
        bool enableHotReload = false;
        bool enableSandbox = true;
        bool enableMetrics = false;
        bool enableProfiling = false;
        bool enableAutoUpdate = false;

        // Performance settings
        size_t maxConcurrentLoads = 4;
        size_t updateIntervalMs = 16;
        size_t maxMemoryMB = 512;
        size_t maxPlugins = 100;

        // Logging settings
        struct Logging {
            std::string level = "info";  // verbose, debug, info, warning, error, fatal
            bool enableConsole = true;
            bool enableFile = true;
            std::string fileFormat = "plugify_%Y%m%d.log";
            size_t maxFileSize = 10 * 1024 * 1024;  // 10MB
            size_t maxFiles = 5;
        } logging;

        // Network settings
        struct Network {
            bool enableRemotePlugins = false;
            std::vector<std::string> trustedRepositories;
            std::string proxyUrl;
            size_t downloadTimeoutSec = 30;
        } network;

        // Security settings
        struct Security {
            bool requireSignedPlugins = false;
            std::vector<std::string> allowedPluginPaths;
            std::vector<std::string> blockedPlugins;
            size_t maxPluginMemoryMB = 100;
        } security;

        // Create from ConfigNode
        static Result<PlugifyConfig> FromConfigNode(const ConfigNode& node);

        // Convert to ConfigNode
        ConfigNode ToConfigNode() const;

        // Load from file with schema validation
        static Result<PlugifyConfig> LoadFromFile(const std::filesystem::path& path);

        // Get default schema
        static ConfigSchema GetSchema();

        // Merge with another config (other takes precedence)
        void Merge(const PlugifyConfig& other);
    };
#endif
} // namespace plugify
