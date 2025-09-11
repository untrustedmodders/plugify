#include "core/glaze_manifest_parser.hpp"
#include "core/conflict_impl.hpp"
#include "core/dependency_impl.hpp"
#include "core/enum_object_impl.hpp"
#include "core/enum_value_impl.hpp"
#include "core/method_impl.hpp"

#include "plugify/file_system.hpp"

using namespace plugify;

namespace {
    // Helper function to validate a name (alphanumeric, underscore)
    bool IsValidName(const std::string& name) {
        if (name.empty()) return false;
        static const std::regex nameRegex(
            "^[a-zA-Z][a-zA-Z0-9_]*$"
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
    Result<void> ValidateEnumObject(const EnumObject::Impl& enumObj) {
        if (enumObj.name.empty()) {
            return MakeError("Enum name cannot be empty");
        }

        if (!IsValidName(enumObj.name)) {
            return MakeError("Invalid enum name: {}", enumObj.name);
        }

        if (enumObj.values.empty()) {
            // TODO: add ref system to enum and prototype to allow easy dublication similar to schema $ref
            return {};
            //return MakeError("Enum '{}' must have at least one value", enumObj.name);
        }

        std::unordered_set<std::string> valueNames;
        std::unordered_set<int64_t> valueNumbers;

        valueNames.reserve(enumObj.values.size());
        valueNumbers.reserve(enumObj.values.size());

        for (const auto& v : enumObj.values) {
            const auto& value = *v._impl;

            if (value.name.empty()) {
                return MakeError("Enum value name cannot be empty in enum '{}'", enumObj.name);
            }

            if (!IsValidName(value.name)) {
                return MakeError("Invalid enum value name: {} in enum '{}'", value.name, enumObj.name);
            }

            if (!valueNames.insert(value.name).second) {
                return MakeError("Duplicate enum value name: {} in enum '{}'", value.name, enumObj.name);
            }

            if (!valueNumbers.insert(value.value).second) {
                return MakeError("Duplicate enum value: {} for '{}' in enum '{}'",
                                   value.value, value.name, enumObj.name);
            }
        }

        return {};
    }

    extern Result<void> ValidateMethod(const Method::Impl& method, const std::string& context = "");

    // Validate Property
    Result<void> ValidateProperty(const Property::Impl& prop, const std::string& context, bool param) {
        // Check if type is valid (you'd need to define valid ValueType values)
        // Assuming ValueType is an enum, check if it's in valid range

        // If type is void, something wrong
        if (param && prop.type == ValueType::Void) {
            return MakeError("{}: Parameter cannot be void type", context);
        }

        // If type is function/callback, prototype should be present
        if (prop.type == ValueType::Function && !prop.prototype) {
            return MakeError("{}: Function type requires prototype", context);
        }

        // If type is enum, enumerate should be present
        // if (prop.type == ValueType::Enum && !prop.enumerate) {
        //     return context + ": Enum type requires enumerate definition";
        // }

        // Validate enum if present
        if (prop.enumerate) {
            if (auto result = ValidateEnumObject(*prop.enumerate->_impl); !result) {
                return MakeError("{}: {}", context, result.error());
            }
        }

        // Validate prototype if present (recursive validation)
        if (prop.prototype) {
            if (auto result = ValidateMethod(*prop.prototype->_impl, std::format("{}{}", context, ".prototype")); !result) {
                return result;
            }
        }

        return {};
    }

    // Validate Method
    Result<void> ValidateMethod(const Method::Impl& method, const std::string& context) {
        std::string prefix = context.empty() ? "Method" : context;

        if (method.name.empty()) {
            return MakeError("{}: name cannot be empty", prefix);
        }

        if (!IsValidName(method.name)) {
            return MakeError("{}: invalid name '{}'", prefix, method.name);
        }

        if (context.empty()) {
            if (method.funcName.empty()) {
                return MakeError("{} '{}': funcName cannot be empty", prefix, method.name);
            }

            if (!IsValidName(method.funcName)) {
                return MakeError("{} '{}': invalid funcName '{}'", prefix, method.name, method.funcName);
            }
        }

        // Validate varIndex
        constexpr uint8_t kNoVarArgs = 255; // Assuming this constant
        if (method.varIndex != kNoVarArgs && method.varIndex >= method.paramTypes.size()) {
            return MakeError("{} '{}': varIndex out of range", prefix, method.name);
        }

        // Validate parameter types
        for (size_t i = 0; i < method.paramTypes.size(); ++i) {
            if (auto result = ValidateProperty(*method.paramTypes[i]._impl,
                std::format("{} '{}' param[{}]", prefix, method.name, i), true); !result) {
                return result;
            }
        }

        // Validate return type
        if (auto result = ValidateProperty(*method.retType._impl,
            std::format("{} '{}' return type", prefix, method.name), false); !result) {
            return result;
        }

        return {};
    }

    // Validate Dependency
    Result<void> ValidateDependency(const Dependency::Impl& dep) {
        if (dep.name.empty()) {
            return MakeError("Dependency name cannot be empty");
        }

        if (!IsValidName(dep.name)) {
            return MakeError("Invalid dependency name: ", dep.name);
        }

        // Validate constraints if present
        // You would need to implement constraint validation based on your Constraint structure
        // if (dep.constraints) {
        //     if (auto result = ValidateConstraint(*dep.constraints); !result) {
        //         return MakeError("Dependency '" + dep.name + "': " + *error);
        //     }
        // }

        return {};
    }

    // Validate Conflict
    Result<void> ValidateConflict(const Conflict::Impl& conflict) {
        if (conflict.name.empty()) {
            return MakeError("Conflict name cannot be empty");
        }

        if (!IsValidName(conflict.name)) {
            return MakeError("Invalid conflict name: ", conflict.name);
        }

        // Validate constraints if present
        // if (conflict.constraints) {
        //     if (auto result = ValidateConstraint(*conflict.constraints); !result) {
        //         return MakeError("Conflict '" + conflict.name + "': " + *error);
        //     }
        // }

        return {};
    }

    // Validate Obsolete
    Result<void> ValidateObsolete(const Obsolete::Impl& obsolete) {
        if (obsolete.name.empty()) {
            return MakeError("Obsolete name cannot be empty");
        }

        if (!IsValidName(obsolete.name)) {
            return MakeError("Invalid obsolete name: ", obsolete.name);
        }

        return {};
    }
}

// Main Manifest validation implementation
Result<void> Manifest::Validate() const {
    // Validate required common fields
    if (name.empty()) {
        return MakeError("Manifest name is required");
    }

    if (!IsValidName(name)) {
        return MakeError("Invalid manifest name: {}", name);
    }

    // Validate version (assuming Version has its own validation)
    // if (!version.IsValid()) {
    //     return MakeError("Invalid version format");
    // }

    if (language.empty()) {
        return MakeError("Language cannot be empty if specified");
    }

    if (!IsValidName(language)) {
        return MakeError("Invalid language name: {}", language);
    }

    // Validate optional common fields
    if (author && author->empty()) {
        return MakeError("Author cannot be empty if specified");
    }

    if (website && !IsValidURL(*website)) {
        return MakeError("Invalid website URL: {}", *website);
    }

    if (license && license->empty()) {
        return MakeError("License cannot be empty if specified");
    }

    // Validate platforms
    if (platforms) {
        for (const auto& platform : *platforms) {
            if (!IsValidPlatform(platform)) {
                return MakeError("Invalid platform: {}", platform);
            }
        }
    }

    // Validate dependencies
    if (dependencies) {
        for (const auto& dep : *dependencies) {
            if (auto result = ValidateDependency(*dep._impl); !result) {
                return result;
            }
        }
    }

    // Validate conflicts
    if (conflicts) {
        for (const auto& conflict : *conflicts) {
            if (auto result = ValidateConflict(*conflict._impl); !result) {
                return result;
            }
        }
    }

    // Validate obsoletes
    if (obsoletes) {
        for (const auto& obsolete : *obsoletes) {
            if (auto result = ValidateObsolete(*obsolete._impl); !result) {
                return result;
            }
        }
    }

    // Determine manifest type based on fields present
    bool hasPluginFields = entry.has_value() || methods.has_value();
    bool hasModuleFields = runtime.has_value() || directories.has_value();

    if (hasPluginFields && hasModuleFields) {
        return MakeError("Manifest cannot have both plugin and module fields");
    }

    // Validate plugin-specific fields
    if (hasPluginFields) {
        if (entry && entry->empty()) {
            return MakeError("Plugin entry point cannot be empty");
        }

        /*if (entry && !IsValidName(*entry)) {
            return MakeError("Invalid plugin entry point: {}", *entry);
        }*/

        if (methods) {
            /*if (methods->empty()) {
                return MakeError("Methods list cannot be empty if specified");
            }*/

            std::unordered_set<std::string> methodNames;
            std::unordered_set<std::string> functionNames;

            methodNames.reserve(methods->size());
            functionNames.reserve(methods->size());

            for (const auto& method : *methods) {
                if (!methodNames.insert(method._impl->name).second) {
                    return MakeError("Duplicate method name: {}", method._impl->name);
                }

                if (!functionNames.insert(method._impl->funcName).second) {
                    return MakeError("Duplicate function name: {}", method._impl->funcName);
                }

                if (auto result = ValidateMethod(*method._impl); !result) {
                    return result;
                }
            }
        }
    }

    // Validate module-specific fields
    if (hasModuleFields) {
        if (runtime) {
            /*if (runtime->empty()) {
                return MakeError("Module runtime path cannot be empty");
            }*/

            // Check if runtime file exists (optional, might want to make this configurable)
            /*if (!fs->IsExists(*runtime)) {
                return MakeError("Runtime file does not exist: {}", plg::as_string(*runtime));
            }

            // Check if it's a regular file
            if (!fs->IsRegularFile(*runtime)) {
                return MakeError("Runtime path is not a file: {}", plg::as_string(*runtime));
            }*/
        }

        if (directories) {
            if (directories->empty()) {
                return MakeError("Directories list cannot be empty if specified");
            }

            for (const auto& dir : *directories) {
                if (dir.empty()) {
                    return MakeError("Directory path cannot be empty");
                }

                // Check if directory exists (optional)
                /*if (!fs->IsExists(dir)) {
                    return MakeError("Directory does not exist: {}", plg::as_string(dir));
                }

                if (!fs->IsDirectory(dir)) {
                    return MakeError("Path is not a directory: {}", plg::as_string(dir));
                }*/
            }
        }
    }

    // All validations passed
    return {};
}
