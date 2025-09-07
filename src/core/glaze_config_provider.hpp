#pragma once

#include "plugify/config_provider.hpp"

#include "core/glaze_metadata.hpp"

namespace plugify {
#if 0
    // Alternative: strongly-typed configuration using Glaze's compile-time reflection
    template<typename ConfigType>
    class TypedGlazeConfigProvider : public IConfigProvider {
    public:
        TypedGlazeConfigProvider() = default;
        ~TypedGlazeConfigProvider() override = default;

        Result<std::any> GetValue(std::string_view key) override {
            std::shared_lock lock(_mutex);

            // For typed config, we need to use Glaze's JSON pointer syntax
            std::string jsonPointer = std::format("/{}", jsonPointer);
            std::replace(jsonPointer.begin(), jsonPointer.end(), '.', '/');

            try {
                glz::json_t json;
                auto buffer = glz::write_json(_config);

                if (buffer) {
                    auto error = glz::read_jsonc(json, buffer.value());
                    if (error) {
                        return MakeError("Configuration key '{}' could not be read: {}", key, glz::format_error(error, buffer));
                    }

                    // Navigate using JSON pointer
                    auto* value = glz::get_if<glz::json_t>(json, jsonPointer);
                    if (!value) {
                        return MakeError("Configuration key '{}' not found", key);
                    }

                    return ConvertGlazeValue(*value);
                }

                return MakeError(
                    "Failed to serialize configuration"
                );
            } catch (const std::exception& e) {
                return MakeError("Failed to get config value: {}", e.what());
            }
        }

        Result<void> SetValue(std::string_view key, std::any value) override {
            std::unique_lock lock(_mutex);

            // For strongly typed config, this is more complex
            // You'd need to use runtime reflection or limit to specific paths
            return MakeError(
                "Setting individual values not supported for typed configuration. "
                "Use SetConfig() to update the entire configuration object."
            );
        }

        Result<void> LoadFromFile(const std::filesystem::path& path) override {
            std::unique_lock lock(_mutex);

            try {
                auto file_result = plg::as_string(glz::read_file_jsonc<ConfigType>(path));

                if (file_result) {
                    _config = std::move(file_result.value());
                    _configPath = path;
                    _isDirty = false;
                    return {};
                }

                return MakeError("Failed to load configuration: {}", file_result.error());
            } catch (const std::exception& e) {
                return MakeError("Failed to load configuration: {}", e.what());
            }
        }

        Result<void> SaveToFile(const std::filesystem::path& path) override {
            std::shared_lock lock(_mutex);

            try {
                // Create directory if needed
                auto parent = path.parent_path();
                if (!parent.empty() && !std::filesystem::exists(parent)) {
                    std::filesystem::create_directories(parent);
                }

                auto result = glz::write_file_json(_config, plg::as_string(path), std::string{});

                if (result) {
                    return MakeError("Failed to save configuration file");
                }

                _isDirty = false;
                return {};
            } catch (const std::exception& e) {
                return MakeError("Failed to save configuration: {}", e.what()));
            }
        }

        // Direct access to typed configuration
        const ConfigType& GetConfig() const {
            std::shared_lock lock(_mutex);
            return _config;
        }

        void SetConfig(ConfigType config) {
            std::unique_lock lock(_mutex);
            _config = std::move(config);
            _isDirty = true;
        }

        bool IsDirty() const override {
            std::shared_lock lock(_mutex);
            return _isDirty;
        }

    private:
        mutable std::shared_mutex _mutex;
        ConfigType _config{};
        std::filesystem::path _configPath;
        mutable bool _isDirty = false;

        static std::any ConvertGlazeValue(const glz::json_t& value) {
            return std::visit([](const auto& val) -> std::any {
                using T = std::decay_t<decltype(val)>;

                if constexpr (std::is_same_v<T, glz::json_t::null_t>) {
                    return std::any{};
                }
                else if constexpr (std::is_same_v<T, bool>) {
                    return val;
                }
                else if constexpr (std::is_same_v<T, glz::json_t::val_t>) {
                    return std::visit([](const auto& num) -> std::any {
                        using NumType = std::decay_t<decltype(num)>;
                        if constexpr (std::is_integral_v<NumType>) {
                            return static_cast<int64_t>(num);
                        } else {
                            return static_cast<double>(num);
                        }
                    }, val);
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    return val;
                }
                else if constexpr (std::is_same_v<T, glz::json_t::array_t>) {
                    std::vector<std::any> vec;
                    for (const auto& item : val) {
                        vec.push_back(ConvertGlazeValue(item));
                    }
                    return vec;
                }
                else if constexpr (std::is_same_v<T, glz::json_t::object_t>) {
                    std::unordered_map<std::string, std::any> map;
                    for (const auto& [k, v] : val) {
                        map[k] = ConvertGlazeValue(v);
                    }
                    return map;
                }
                else {
                    return std::any{};
                }
            }, value.data);
        }
    };

    // Configuration value types that we support
    using ConfigValue = std::variant<
        std::monostate,  // null
        bool,
        int64_t,
        double,
        std::string,
        std::vector<std::any>,
        std::unordered_map<std::string, std::any>
    >;

    // Generic configuration node that can hold any JSON value
    struct ConfigNode {
        std::unordered_map<std::string, std::any> data;

        // Helper to convert to/from Glaze's JSON value type
        glz::json_t ToJson() const {
            return ConvertToJson(std::any(data));
        }

        void FromJson(const glz::json_t& json) {
            data = ConvertFromJson(json);
        }

    private:
        static glz::json_t ConvertToJson(const std::any& value) {
            if (!value.has_value()) {
                return nullptr;
            }

            // Try to cast to known types
            if (auto* b = std::any_cast<bool>(&value)) {
                return *b;
            }
            if (auto* i = std::any_cast<int>(&value)) {
                return static_cast<int64_t>(*i);
            }
            if (auto* i32 = std::any_cast<int32_t>(&value)) {
                return static_cast<int64_t>(*i32);
            }
            if (auto* i64 = std::any_cast<int64_t>(&value)) {
                return *i64;
            }
            if (auto* f = std::any_cast<float>(&value)) {
                return static_cast<double>(*f);
            }
            if (auto* d = std::any_cast<double>(&value)) {
                return *d;
            }
            if (auto* s = std::any_cast<std::string>(&value)) {
                return *s;
            }
            if (auto* sv = std::any_cast<std::string_view>(&value)) {
                return std::string(*sv);
            }

            // Handle containers
            if (auto* vec = std::any_cast<std::vector<std::any>>(&value)) {
                glz::json_t::array_t arr;
                for (const auto& item : *vec) {
                    arr.push_back(ConvertToJson(item));
                }
                return arr;
            }

            if (auto* map = std::any_cast<std::unordered_map<std::string, std::any>>(&value)) {
                glz::json_t::object_t obj;
                for (const auto& [key, val] : *map) {
                    obj[key] = ConvertToJson(val);
                }
                return obj;
            }

            // Default to null for unknown types
            return nullptr;
        }

        static std::unordered_map<std::string, std::any> ConvertFromJson(const glz::json_t& json) {
            std::unordered_map<std::string, std::any> result;

            if (auto* obj = std::get_if<glz::json_t::object_t>(&json.data)) {
                for (const auto& [key, value] : *obj) {
                    result[key] = ConvertJsonValue(value);
                }
            }

            return result;
        }

        static std::any ConvertJsonValue(const glz::json_t& json) {
            return std::visit([](const auto& val) -> std::any {
                using T = std::decay_t<decltype(val)>;

                if constexpr (std::is_same_v<T, glz::json_t::null_t>) {
                    return std::any{};
                }
                else if constexpr (std::is_same_v<T, bool>) {
                    return val;
                }
                else if constexpr (std::is_same_v<T, glz::json_t::val_t>) {
                    // Glaze uses a variant for numbers, we need to extract the actual type
                    return std::visit([](const auto& num) -> std::any {
                        using NumType = std::decay_t<decltype(num)>;
                        if constexpr (std::is_integral_v<NumType>) {
                            return static_cast<int64_t>(num);
                        } else {
                            return static_cast<double>(num);
                        }
                    }, val);
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    return val;
                }
                else if constexpr (std::is_same_v<T, glz::json_t::array_t>) {
                    std::vector<std::any> vec;
                    for (const auto& item : val) {
                        vec.push_back(ConvertJsonValue(item));
                    }
                    return vec;
                }
                else if constexpr (std::is_same_v<T, glz::json_t::object_t>) {
                    std::unordered_map<std::string, std::any> map;
                    for (const auto& [k, v] : val) {
                        map[k] = ConvertJsonValue(v);
                    }
                    return map;
                }
                else {
                    return std::any{};
                }
            }, json.data);
        }
    };

    class GlazeConfigProvider : public IConfigProvider {
    public:
        GlazeConfigProvider() = default;
        ~GlazeConfigProvider() = default;

        Result<std::any> GetValue(std::string_view key) override {
            std::shared_lock lock(_mutex);

            try {
                auto keys = SplitKey(key);

                // Navigate through the configuration tree
                const std::any* current = nullptr;
                const std::unordered_map<std::string, std::any>* currentMap = &_configData;

                for (size_t i = 0; i < keys.size(); ++i) {
                    auto it = currentMap->find(keys[i]);
                    if (it == currentMap->end()) {
                        return MakeError("Configuration key '{}' not found", key);
                    }

                    current = &it->second;

                    // If not the last key, we need to navigate deeper
                    if (i < keys.size() - 1) {
                        currentMap = std::any_cast<std::unordered_map<std::string, std::any>>(current);
                        if (!currentMap) {
                            return MakeError("Configuration path '{}' is not an object", JoinKeys(keys, i)));
                        }
                    }
                }

                return *current;
            } catch (const std::exception& e) {
                return MakeError("Failed to get config value: {}", e.what());
            }
        }

        Result<void> SetValue(std::string_view key, std::any value) override {
            std::unique_lock lock(_mutex);

            try {
                auto keys = SplitKey(key);
                if (keys.empty()) {
                    return MakeError("Empty configuration key");
                }

                // Navigate to the parent object, creating path if needed
                std::unordered_map<std::string, std::any>* current = &_configData;

                for (size_t i = 0; i < keys.size() - 1; ++i) {
                    auto& next = (*current)[keys[i]];

                    // Create intermediate object if it doesn't exist
                    if (!next.has_value()) {
                        next = std::unordered_map<std::string, std::any>{};
                    }

                    // Get pointer to the map
                    try {
                        current = std::any_cast<std::unordered_map<std::string, std::any>>(&next);
                        if (!current) {
                            return MakeError("Configuration path '{}' is not an object", JoinKeys(keys, i));
                        }
                    } catch (const std::bad_any_cast&) {
                        return MakeError("Configuration path '{}' is not an object", JoinKeys(keys, i));
                    }
                }

                // Set the value
                (*current)[keys.back()] = std::move(value);
                _isDirty = true;

                return {};
            } catch (const std::exception& e) {
                return MakeError("Failed to set config value: {}", e.what());
            }
        }

        Result<void> LoadFromFile(const std::filesystem::path& path) override {
            std::unique_lock lock(_mutex);

            try {
                if (!std::filesystem::exists(path)) {
                    return MakeError("Configuration file '{}' not found", plg::as_string(path));
                }

                // Read file contents
                std::ifstream file(path, std::ios::binary | std::ios::ate);
                if (!file) {
                    return MakeError("Failed to open configuration file '{}'", plg::as_string(path));
                }

                auto size = file.tellg();
                file.seekg(0, std::ios::beg);

                std::string buffer(size, '\0');
                if (!file.read(buffer.data(), size)) {
                    return MakeError("Failed to read configuration file '{}'", plg::as_string(path));
                }

                // Parse JSON with Glaze
                glz::json_t json{};
                auto parse_result = glz::read_jsonc(json, buffer);

                if (parse_result) {
                    return MakeError("Failed to parse JSON: {}", glz::format_error(parse_result, buffer));
                }

                // Convert to our internal format
                ConfigNode node;
                node.FromJson(json);
                _configData = std::move(node.data);
                _configPath = path;
                _isDirty = false;

                return {};
            } catch (const std::exception& e) {
                return MakeError("Failed to load configuration: {}", e.what());
            }
        }

        Result<void> SaveToFile(const std::filesystem::path& path) override {
            std::shared_lock lock(_mutex);

            try {
                // Create directory if it doesn't exist
                auto parent = path.parent_path();
                if (!parent.empty() && !std::filesystem::exists(parent)) {
                    std::filesystem::create_directories(parent);
                }

                // Convert to Glaze JSON
                ConfigNode node;
                node.data = _configData;
                glz::json_t json = node.ToJson();

                // Write to string with pretty printing
                std::string buffer;
                auto write_result = glz::write_json(json, buffer);

                if (write_result) {
                    return MakeError("Failed to serialize configuration to JSON");
                }

                // Pretty print the JSON
                std::string pretty_json;
                /*auto prettify_result = */glz::prettify_json(buffer, pretty_json);

                /*if (prettify_result) {
                    // If prettify fails, use the original buffer
                    pretty_json = buffer;
                }*/

                // Write to file
                std::ofstream file(path, std::ios::binary);
                if (!file) {
                    return MakeError("Failed to create configuration file '{}'", plg::as_string(path));
                }

                file.write(pretty_json.data(), pretty_json.size());

                if (!file) {
                    return MakeError("Failed to write configuration file '{}'", plg::as_string(path));
                }

                _isDirty = false;

                return {};
            } catch (const std::exception& e) {
                return MakeError("Failed to save configuration: {}", e.what());
            }
        }

        // Additional helper methods
        bool IsDirty() const override {
            std::shared_lock lock(_mutex);
            return _isDirty;
        }

        void Clear() {
            std::unique_lock lock(_mutex);
            _configData.clear();
            _isDirty = true;
        }

        // Load defaults from a structured type (using Glaze reflection)
        template<typename T>
        void LoadDefaults(const T& defaults) {
            std::unique_lock lock(_mutex);

            std::string buffer;
            auto result = glz::write_json(defaults, buffer);
            if (!result) {
                glz::json_t json;
                if (!glz::read_jsonc(json, buffer)) {
                    ConfigNode node;
                    node.FromJson(json);
                    MergeDefaults(_configData, node.data);
                }
            } else {

            }
        }

        // Type-safe getters for common types
        template<typename T>
        Result<T> GetTyped(std::string_view key) {
            auto result = GetValue(key);
            if (!result) {
                return MakeError(std::move(result.error()));
            }

            try {
                if (auto* value = std::any_cast<T>(&result.value())) {
                    return *value;
                }

                // Try numeric conversions
                if constexpr (std::is_arithmetic_v<T>) {
                    if (auto* i = std::any_cast<int64_t>(&result.value())) {
                        return static_cast<T>(*i);
                    }
                    if (auto* d = std::any_cast<double>(&result.value())) {
                        return static_cast<T>(*d);
                    }
                }

                return MakeError("Configuration value '{}' has incorrect type", key);
            } catch (const std::exception& e) {
                return MakeError("Failed to convert configuration value: {}", e.what());
            }
        }

        // Convenience methods for common types
        Result<bool> GetBool(std::string_view key) { return GetTyped<bool>(key); }
        Result<int64_t> GetInt(std::string_view key) { return GetTyped<int64_t>(key); }
        Result<double> GetDouble(std::string_view key) { return GetTyped<double>(key); }
        Result<std::string> GetString(std::string_view key) { return GetTyped<std::string>(key); }

        // Get with default value
        template<typename T>
        T GetOr(std::string_view key, T defaultValue) {
            auto result = GetTyped<T>(key);
            return result ? result.value() : defaultValue;
        }

        // Batch operations
        Result<void> SetMultiple(const std::unordered_map<std::string, std::any>& values) {
            for (const auto& [key, value] : values) {
                if (auto result = SetValue(key, value); !result) {
                    return result;
                }
            }
            return {};
        }

        // Watch for changes (simplified version)
        using ChangeCallback = std::function<void(std::string_view key, const std::any& newValue)>;

        void OnChange(std::string_view keyPrefix, ChangeCallback callback) {
            std::unique_lock lock(_mutex);
            _changeCallbacks[std::string(keyPrefix)].push_back(callback);
        }

    private:
        mutable std::shared_mutex _mutex;
        std::unordered_map<std::string, std::any> _configData;
        std::filesystem::path _configPath;
        mutable bool _isDirty = false;
        std::unordered_map<std::string, std::vector<ChangeCallback>> _changeCallbacks;

        static std::vector<std::string> SplitKey(std::string_view key) {
            std::vector<std::string> parts;
            size_t start = 0;
            size_t end = key.find('.');

            while (end != std::string_view::npos) {
                parts.emplace_back(key.substr(start, end - start));
                start = end + 1;
                end = key.find('.', start);
            }

            if (start < key.length()) {
                parts.emplace_back(key.substr(start));
            }

            return parts;
        }

        static std::string JoinKeys(const std::vector<std::string>& keys, size_t upTo) {
            std::string result;
            for (size_t i = 0; i <= upTo && i < keys.size(); ++i) {
                if (!result.empty()) result += '.';
                result += keys[i];
            }
            return result;
        }

        static void MergeDefaults(std::unordered_map<std::string, std::any>& target,
                                  const std::unordered_map<std::string, std::any>& defaults) {
            for (const auto& [key, value] : defaults) {
                if (!target.contains(key)) {
                    target[key] = value;
                } else if (auto* targetMap = std::any_cast<std::unordered_map<std::string, std::any>>(&target[key])) {
                    if (auto* defaultMap = std::any_cast<std::unordered_map<std::string, std::any>>(&value)) {
                        MergeDefaults(*targetMap, *defaultMap);
                    }
                }
            }
        }

        void NotifyCallbacks(std::string_view key, const std::any& newValue) {
            // Find all callbacks that match this key or its parents
            for (const auto& [prefix, callbacks] : _changeCallbacks) {
                if (key.starts_with(prefix)) {
                    for (const auto& callback : callbacks) {
                        callback(key, newValue);
                    }
                }
            }
        }
    };
#endif
}