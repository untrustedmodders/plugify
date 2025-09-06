#include "core/glaze_manifest_parser.hpp"
#include "core/conflict_impl.hpp"
#include "core/dependency_impl.hpp"
#include "core/enum_object_impl.hpp"
#include "core/enum_value_impl.hpp"
#include "core/method_impl.hpp"

#include "plugify/file_system.hpp"

using namespace plugify;

namespace {
    // Helper function to validate a name (alphanumeric, underscore, dash, dot)
    bool IsValidName(const std::string& name) {
        if (name.empty()) return false;
        static const std::regex nameRegex(
            "^[a-zA-Z][a-zA-Z0-9_.-]*$"
        );
        return std::regex_match(name, nameRegex);
    }

    // Helper function to validate URL
    bool IsValidURL(const std::string& url) {
        static const std::regex urlRegex(
            R"(^(https?://)?([a-zA-Z0-9-]+\.)+[a-zA-Z]{2,}(/.*)?$)"
        );
        return std::regex_match(url, urlRegex);
    }

    // Helper function to validate platform
    bool IsValidPlatform(const std::string& platform) {
        /*static const std::unordered_set<std::string> validPlatforms = {
            "windows", "linux", "macos", "android", "ios",
            "unix", "posix", "win32", "win64", "x86", "x64", "arm", "arm64"
        };*/
        if (platform.empty()) return true;
        static const std::regex platformRegex(
            "^[a-z0-9]+(_(x64|arm64|x86|arm32|riscv32|riscv64))$"
        );
        return std::regex_match(platform, platformRegex);
    }

    // Validate EnumObject
    std::optional<std::string> ValidateEnumObject(const EnumObject::Impl& enumObj) {
        if (enumObj.name.empty()) {
            return std::format("Enum name cannot be empty");
        }

        if (!IsValidName(enumObj.name)) {
            return std::format("Invalid enum name: {}", enumObj.name);
        }

        if (enumObj.values.empty()) {
            return std::format("Enum '{}' must have at least one value", enumObj.name);
        }

        std::unordered_set<std::string> valueNames;
        std::unordered_set<int64_t> valueNumbers;

        valueNames.reserve(enumObj.values.size());
        valueNumbers.reserve(enumObj.values.size());

        for (const auto& v : enumObj.values) {
            const auto& value = *v._impl;

            if (value.name.empty()) {
                return std::format("Enum value name cannot be empty in enum '{}'", enumObj.name);
            }

            if (!IsValidName(value.name)) {
                return std::format("Invalid enum value name: {} in enum '{}'", value.name, enumObj.name);
            }

            if (!valueNames.insert(value.name).second) {
                return std::format("Duplicate enum value name: {} in enum '{}'", value.name, enumObj.name);
            }

            if (!valueNumbers.insert(value.value).second) {
                return std::format("Duplicate enum value: {} for '{}' in enum '{}'",
                                   value.value, value.name, enumObj.name);
            }
        }

        return std::nullopt;
    }

    extern std::optional<std::string> ValidateMethod(const Method::Impl& method, const std::string& context = "");

    // Validate Property
    std::optional<std::string> ValidateProperty(const Property::Impl& prop, const std::string& context, bool param) {
        // Check if type is valid (you'd need to define valid ValueType values)
        // Assuming ValueType is an enum, check if it's in valid range

        // If type is void, something wrong
        if (param && prop.type == ValueType::Void) {
            return std::format("{}: Parameter cannot be void type", context);
        }

        // If type is function/callback, prototype should be present
        if (prop.type == ValueType::Function && !prop.prototype) {
            return std::format("{}: Function type requires prototype", context);
        }

        // If type is enum, enumerate should be present
        // if (prop.type == ValueType::Enum && !prop.enumerate) {
        //     return context + ": Enum type requires enumerate definition";
        // }

        // Validate enum if present
        if (prop.enumerate) {
            if (auto error = ValidateEnumObject(*prop.enumerate->_impl)) {
                return std::format("{}: {}", context, *error);
            }
        }

        // Validate prototype if present (recursive validation)
        if (prop.prototype) {
            if (auto error = ValidateMethod(*prop.prototype->_impl, std::format("{}{}", context, ".prototype"))) {
                return error;
            }
        }

        return std::nullopt;
    }

    // Validate Method
    std::optional<std::string> ValidateMethod(const Method::Impl& method, const std::string& context) {
        std::string prefix = context.empty() ? "Method" : context;

        if (method.name.empty()) {
            return std::format("{}: name cannot be empty", prefix);
        }

        if (!IsValidName(method.name)) {
            return std::format("{}: invalid name '{}'", prefix, method.name);
        }

        if (method.funcName.empty()) {
            return std::format("{} '{}': funcName cannot be empty", prefix, method.name);
        }

        if (!IsValidName(method.funcName)) {
            return std::format("{} '{}': invalid funcName '{}'", prefix, method.name, method.funcName);
        }

        // Validate varIndex
        constexpr uint8_t kNoVarArgs = 255; // Assuming this constant
        if (method.varIndex != kNoVarArgs && method.varIndex >= method.paramTypes.size()) {
            return std::format("{} '{}': varIndex out of range", prefix, method.name);
        }

        // Validate parameter types
        for (size_t i = 0; i < method.paramTypes.size(); ++i) {
            if (auto error = ValidateProperty(*method.paramTypes[i]._impl,
                std::format("{} '{}' param[{}]", prefix, method.name, i), true)) {
                return error;
            }
        }

        // Validate return type
        if (auto error = ValidateProperty(*method.retType._impl,
            std::format("{} '{}' return type", prefix, method.name), false)) {
            return error;
        }

        return std::nullopt;
    }

    // Validate Dependency
    std::optional<std::string> ValidateDependency(const Dependency::Impl& dep) {
        if (dep.name.empty()) {
            return "Dependency name cannot be empty";
        }

        if (!IsValidName(dep.name)) {
            return std::format("Invalid dependency name: ", dep.name);
        }

        // Validate constraints if present
        // You would need to implement constraint validation based on your Constraint structure
        // if (dep.constraints) {
        //     if (auto error = ValidateConstraint(*dep.constraints)) {
        //         return "Dependency '" + dep.name + "': " + *error;
        //     }
        // }

        return std::nullopt;
    }

    // Validate Conflict
    std::optional<std::string> ValidateConflict(const Conflict::Impl& conflict) {
        if (conflict.name.empty()) {
            return "Conflict name cannot be empty";
        }

        if (!IsValidName(conflict.name)) {
            return std::format("Invalid conflict name: ", conflict.name);
        }

        // Validate constraints if present
        // if (conflict.constraints) {
        //     if (auto error = ValidateConstraint(*conflict.constraints)) {
        //         return "Conflict '" + conflict.name + "': " + *error;
        //     }
        // }

        return std::nullopt;
    }

    // Validate Obsolete
    std::optional<std::string> ValidateObsolete(const Obsolete::Impl& obsolete) {
        if (obsolete.name.empty()) {
            return "Obsolete name cannot be empty";
        }

        if (!IsValidName(obsolete.name)) {
            return std::format("Invalid obsolete name: ", obsolete.name);
        }

        return std::nullopt;
    }
}

// Main Manifest validation implementation
std::optional<std::string> Manifest::Validate() const {
    // Validate required common fields
    if (name.empty()) {
        return "Manifest name is required";
    }

    if (!IsValidName(name)) {
        return std::format("Invalid manifest name: {}", name);
    }

    // Validate version (assuming Version has its own validation)
    // if (!version.IsValid()) {
    //     return "Invalid version format";
    // }

    if (language.empty()) {
        return "Language cannot be empty if specified";
    }

    if (!IsValidName(language)) {
        return std::format("Invalid language name: {}", language);
    }

    // Validate optional common fields
    if (author && author->empty()) {
        return "Author cannot be empty if specified";
    }

    if (website && !IsValidURL(*website)) {
        return std::format("Invalid website URL: {}", *website);
    }

    if (license && license->empty()) {
        return "License cannot be empty if specified";
    }

    // Validate platforms
    if (platforms) {
        for (const auto& platform : *platforms) {
            if (!IsValidPlatform(platform)) {
                return std::format("Invalid platform: {}", platform);
            }
        }
    }

    // Validate dependencies
    if (dependencies) {
        for (const auto& dep : *dependencies) {
            if (auto error = ValidateDependency(*dep._impl)) {
                return error;
            }
        }
    }

    // Validate conflicts
    if (conflicts) {
        for (const auto& conflict : *conflicts) {
            if (auto error = ValidateConflict(*conflict._impl)) {
                return error;
            }
        }
    }

    // Validate obsoletes
    if (obsoletes) {
        for (const auto& obsolete : *obsoletes) {
            if (auto error = ValidateObsolete(*obsolete._impl)) {
                return error;
            }
        }
    }

    // Determine manifest type based on fields present
    bool hasPluginFields = entry.has_value() || methods.has_value();
    bool hasModuleFields = runtime.has_value() || directories.has_value();

    if (hasPluginFields && hasModuleFields) {
        return "Manifest cannot have both plugin and module fields";
    }

    // Validate plugin-specific fields
    if (hasPluginFields) {
        if (entry && entry->empty()) {
            return "Plugin entry point cannot be empty";
        }

        if (entry && !IsValidName(*entry)) {
            return std::format("Invalid plugin entry point: {}", *entry);
        }

        if (methods) {
            if (methods->empty()) {
                return "Methods list cannot be empty if specified";
            }

            std::unordered_set<std::string> methodNames;
            std::unordered_set<std::string> functionNames;

            methodNames.reserve(methods->size());
            functionNames.reserve(methods->size());

            for (const auto& method : *methods) {
                if (!methodNames.insert(method._impl->name).second) {
                    return std::format("Duplicate method name: {}", method._impl->name);
                }

                if (!functionNames.insert(method._impl->funcName).second) {
                    return std::format("Duplicate function name: {}", method._impl->funcName);
                }

                if (auto error = ValidateMethod(*method._impl)) {
                    return error;
                }
            }
        }
    }

    // Validate module-specific fields
    if (hasModuleFields) {
        if (runtime) {
            if (runtime->empty()) {
                return "Module runtime path cannot be empty";
            }

            // Check if runtime file exists (optional, might want to make this configurable)
            /*if (!fs->IsExists(*runtime)) {
                return std::format("Runtime file does not exist: {}", plg::as_string(*runtime));
            }

            // Check if it's a regular file
            if (!fs->IsRegularFile(*runtime)) {
                return std::format("Runtime path is not a file: {}", plg::as_string(*runtime));
            }*/
        }

        if (directories) {
            if (directories->empty()) {
                return "Directories list cannot be empty if specified";
            }

            for (const auto& dir : *directories) {
                if (dir.empty()) {
                    return "Directory path cannot be empty";
                }

                // Check if directory exists (optional)
                /*if (!fs->IsExists(dir)) {
                    return std::format("Directory does not exist: {}", plg::as_string(dir));
                }

                if (!fs->IsDirectory(dir)) {
                    return std::format("Path is not a directory: {}", plg::as_string(dir));
                }*/
            }
        }
    }

    // All validations passed
    return std::nullopt;
}
