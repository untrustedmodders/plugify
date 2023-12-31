#include <glaze/glaze.hpp>
#include <glaze/core/macros.hpp>

template<>
struct glz::meta<wizard::ValueType> {
    using enum wizard::ValueType;
    static constexpr auto value = enumerate(
        "invalid", Invalid,
        "void", Void,
        "bool", Bool,
        "char8", Char8,
        "char16", Char16,
        "int8", Int8,
        "int16", Int16,
        "int32", Int32,
        "int64", Int64,
        "uint8", Uint8,
        "uint16", Uint16,
        "uint32", Uint32,
        "uint64", Uint64,
        "ptr64", Ptr64,
        "float", Float,
        "double", Double,
        "string", String
    );
};

GLZ_META(wizard::PluginDescriptor, fileVersion, version, versionName, friendlyName, description, createdBy,createdByURL, docsURL, downloadURL, supportURL, assemblyPath, supportedPlatforms, languageModule, dependencies, exportedMethods);

namespace glz::detail {
    template <>
    struct from_json<fs::path> {
        template <auto Opts>
        static void op(fs::path& value, auto&&... args) {
            std::string str;
            read<json>::op<Opts>(str, args...);
            value = str;
            if (!value.empty()) value.make_preferred();
        }
    };

    template <>
    struct to_json<fs::path> {
        template <auto Opts>
        static void op(fs::path& value, auto&&... args) noexcept {
            write<json>::op<Opts>(value.generic_string(), args...);
        }
    };
}