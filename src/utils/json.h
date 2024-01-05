#pragma once

#include <glaze/glaze.hpp>
#include <glaze/core/macros.hpp>

#include <wizard/method.h>
#include <wizard/plugin_descriptor.h>
#include <wizard/config.h>
#include <core/package.h>

template<>
struct glz::meta<wizard::ValueType> {
    static constexpr auto value = enumerate(
        "invalid", wizard::ValueType::Invalid,
        "void", wizard::ValueType::Void,
        "bool", wizard::ValueType::Bool,
        "char8", wizard::ValueType::Char8,
        "char16", wizard::ValueType::Char16,
        "int8", wizard::ValueType::Int8,
        "int16", wizard::ValueType::Int16,
        "int32", wizard::ValueType::Int32,
        "int64", wizard::ValueType::Int64,
        "uint8", wizard::ValueType::Uint8,
        "uint16", wizard::ValueType::Uint16,
        "uint32", wizard::ValueType::Uint32,
        "uint64", wizard::ValueType::Uint64,
        "ptr64", wizard::ValueType::Ptr64,
        "float", wizard::ValueType::Float,
        "double", wizard::ValueType::Double,
        "string", wizard::ValueType::String,
        "function", wizard::ValueType::Function
    );
};

template<>
struct glz::meta<wizard::Severity> {
    static constexpr auto value = enumerate(
        "none", wizard::Severity::None,
        "fatal", wizard::Severity::Fatal,
        "error", wizard::Severity::Error,
        "warning", wizard::Severity::Warning,
        "info", wizard::Severity::Info,
        "debug", wizard::Severity::Debug,
        "verbose", wizard::Severity::Verbose
    );
};

GLZ_META(wizard::PluginDescriptor, fileVersion, version, versionName, friendlyName, description, createdBy,createdByURL, docsURL, downloadURL, supportURL, assemblyPath, supportedPlatforms, languageModule, dependencies, exportedMethods);
GLZ_META(wizard::Config, baseDir, logSeverity, strictMode, packageVerification, packageVerifyUrl);
GLZ_META(wizard::Package, name, url, version, extractArchive, languageModule);

template <>
struct glz::meta<wizard::Property> {
	using T = wizard::Property;
	static constexpr auto value = object("type", &T::type, "name", skip{}, "paramTypes", skip{}, "retType", skip{});
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