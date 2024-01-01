#pragma once

#include <glaze/glaze.hpp>
#include <glaze/core/macros.hpp>

#include <wizard/method.h>
#include <wizard/plugin_descriptor.h>
#include <wizard/config.h>

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

template<>
struct glz::meta<wizard::Severity> {
    using enum wizard::Severity;
    static constexpr auto value = enumerate(
        "none", None,
        "fatal", Fatal,
        "error", Error,
        "warning", Warning,
        "info", Info,
        "debug", Debug,
        "verbose", Verbose
    );
};

GLZ_META(wizard::PluginDescriptor, fileVersion, version, versionName, friendlyName, description, createdBy,createdByURL, docsURL, downloadURL, supportURL, assemblyPath, supportedPlatforms, languageModule, dependencies, exportedMethods);
GLZ_META(wizard::Config, baseDir, logSeverity, strictMode);

template <>
struct glz::meta<wizard::Property> {
	using T = wizard::Property;
	static constexpr auto value = object("type", &T::type, "name", skip{}, &T::name);
};

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