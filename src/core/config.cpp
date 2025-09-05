#include "plugify/config.hpp"

#if 0
using namespace plugify;

// JSON Reader Implementation
Result<ConfigNode> JsonConfigReader::Read(const std::filesystem::path& path) {
    try {
        std::ifstream file(path);
        if (!file) {
            return MakeError("Failed to open file: {}", plg::as_string(path));
        }

        nlohmann::json j;
        file >> j;

        // Convert JSON to ConfigNode
        std::function<ConfigNode(const nlohmann::json&)> convert;
        convert = [&](const nlohmann::json& json) -> ConfigNode {
            ConfigNode node;

            if (json.is_null()) {
                node = ConfigNode(std::monostate{});
            } else if (json.is_boolean()) {
                node = ConfigNode(json.get<bool>());
            } else if (json.is_number_integer()) {
                node = ConfigNode(json.get<int64_t>());
            } else if (json.is_number_float()) {
                node = ConfigNode(json.get<double>());
            } else if (json.is_string()) {
                node = ConfigNode(json.get<std::string>());
            } else if (json.is_array()) {
                std::vector<std::any> arr;
                for (const auto& item : json) {
                    arr.push_back(convert(item));
                }
                node = ConfigNode(arr);
            } else if (json.is_object()) {
                for (auto& [key, value] : json.items()) {
                    node.SetNode(key, convert(value));
                }
            }

            return node;
        };

        return convert(j);
        return {};
    } catch (const std::exception& e) {
        return MakeError("Failed to parse JSON: {}", e.what());
    }
}

// ConfigManager Implementation
ConfigManager::ConfigManager() {
    // Register default readers
    RegisterReader(std::make_unique<JsonConfigReader>());
    //RegisterReader(std::make_unique<YamlConfigReader>());
    //RegisterReader(std::make_unique<TomlConfigReader>());
    //RegisterReader(std::make_unique<IniConfigReader>());
}

void ConfigManager::RegisterReader(std::unique_ptr<IConfigReader> reader) {
    _readers.emplace_back(std::move(reader));
}

Result<void> ConfigManager::LoadFromFile(const std::filesystem::path& path,
                                        ConfigPriority priority) {
    // Find appropriate reader
    for (const auto& reader : _readers) {
        if (reader->CanRead(path)) {
            auto result = reader->Read(path);
            if (!result) {
                return std::unexpected(result.error());
            }

            // Merge with existing config
            _config.Merge(*result, priority);
            _loadedFiles.push_back(path);
            _metadata.lastReload = std::chrono::system_clock::now();

            // Validate if schema is set
            if (_metadata.hasSchema) {
                return Validate();
            }

            return {};
        }
    }

    return MakeError("No reader found for file: {}", plg::as_string(path));
}

Result<void> ConfigManager::LoadConfigFiles(const std::vector<std::filesystem::path>& paths) {
    // Load files in order, each overriding the previous
    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            if (auto result = LoadFromFile(path); !result) {
                return result;
            }
        }
    }
    return {};
}

Result<void> ConfigManager::LoadFromEnvironment(const EnvironmentConfigSource& source) {
    auto envConfig = source.Read();
    _config.Merge(envConfig, ConfigPriority::Environment);
    return {};
}

std::string ConfigManager::InterpolateString(std::string_view str) const {
    std::string result(str);
    size_t pos = 0;

    while ((pos = result.find("${", pos)) != std::string::npos) {
        size_t endPos = result.find('}', pos);
        if (endPos == std::string::npos) break;

        std::string varPath = result.substr(pos + 2, endPos - pos - 2);

        // Try to get the value
        if (auto value = Get<std::string>(varPath)) {
            result.replace(pos, endPos - pos + 1, *value);
            pos += value->length();
        } else {
            // Try environment variable as fallback
            if (const char* envVal = std::getenv(varPath.c_str())) {
                result.replace(pos, endPos - pos + 1, envVal);
                pos += std::strlen(envVal);
            } else {
                pos = endPos + 1;
            }
        }
    }

    return result;
}

// PlugifyConfig Implementation
ConfigSchema PlugifyConfig::GetSchema() {
    ConfigSchema schema;

    // Define base directories
    schema.AddField({
        .name = "baseDir",
        .type = "string",
        .required = false,
        .defaultValue = plg::as_string(std::filesystem::current_path()),
        .description = "Base directory for all Plugify operations"
    });

    // Define feature flags
    schema.AddField({
        .name = "enableHotReload",
        .type = "bool",
        .required = false,
        .defaultValue = false,
        .description = "Enable hot-reloading of plugins"
    });

    schema.AddField({
        .name = "maxMemoryMB",
        .type = "int",
        .required = false,
        .defaultValue = int64_t(512),
        .description = "Maximum memory usage in MB",
        .range = std::make_pair(ConfigValue(static_cast<int64_t>(64)), ConfigValue(static_cast<int64_t>(8192)))
    });

    // Logging sub-schema
    ConfigSchema loggingSchema;
    loggingSchema.AddField({
        .name = "level",
        .type = "string",
        .required = false,
        .defaultValue = std::string("info"),
        .allowedValues = std::vector<ConfigValue>{
            ConfigValue(std::string("verbose")),
            ConfigValue(std::string("debug")),
            ConfigValue(std::string("info")),
            ConfigValue(std::string("warning")),
            ConfigValue(std::string("error")),
            ConfigValue(std::string("fatal"))
        }
    });

    schema.AddSubSchema("logging", loggingSchema);

    return schema;
}

Result<PlugifyConfig> PlugifyConfig::LoadFromFile(const std::filesystem::path& path) {
    ConfigManager manager;
    manager.SetSchema(GetSchema());

    if (auto result = manager.LoadFromFile(path); !result) {
        return std::unexpected(result.error());
    }

    PlugifyConfig config;

    // Parse configuration
    if (auto val = manager.Get<std::string>("baseDir")) {
        config.baseDir = *val;
    }

    if (auto val = manager.Get<bool>("enableHotReload")) {
        config.enableHotReload = *val;
    }

    if (auto val = manager.Get<int64_t>("maxMemoryMB")) {
        config.maxMemoryMB = static_cast<size_t>(*val);
    }

    if (auto val = manager.Get<std::string>("logging.level")) {
        config.logging.level = *val;
    }

    // Resolve paths relative to baseDir
    config.pluginsDir = config.baseDir / manager.GetOr<std::string>("pluginsDir", "plugins");
    config.configsDir = config.baseDir / manager.GetOr<std::string>("configsDir", "configs");
    config.dataDir = config.baseDir / manager.GetOr<std::string>("dataDir", "data");
    config.logsDir = config.baseDir / manager.GetOr<std::string>("logsDir", "logs");
    config.cacheDir = config.baseDir / manager.GetOr<std::string>("cacheDir", "cache");

    return config;
}

#endif