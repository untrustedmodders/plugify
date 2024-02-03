#pragma once

#include <glaze/glaze.hpp>
#include <glaze/core/macros.hpp>

#include <plugify/method.h>
#include <plugify/descriptor.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/language_module_descriptor.h>
#include <plugify/config.h>
#include <plugify/package.h>

template<>
struct glz::meta<plugify::ValueType> {
    static constexpr auto value = enumerate(
        "invalid", plugify::ValueType::Invalid,
        "void", plugify::ValueType::Void,
        "bool", plugify::ValueType::Bool,
        "char8", plugify::ValueType::Char8,
        "char16", plugify::ValueType::Char16,
        "int8", plugify::ValueType::Int8,
        "int16", plugify::ValueType::Int16,
        "int32", plugify::ValueType::Int32,
        "int64", plugify::ValueType::Int64,
        "uint8", plugify::ValueType::Uint8,
        "uint16", plugify::ValueType::Uint16,
        "uint32", plugify::ValueType::Uint32,
        "uint64", plugify::ValueType::Uint64,
        "ptr64", plugify::ValueType::Ptr64,
        "float", plugify::ValueType::Float,
        "double", plugify::ValueType::Double,
        "string", plugify::ValueType::String,
        "function", plugify::ValueType::Function,
		"bool*", plugify::ValueType::ArrayBool,
		"char8*", plugify::ValueType::ArrayChar8,
		"char16*", plugify::ValueType::ArrayChar16,
		"int8*", plugify::ValueType::ArrayInt8,
		"int16*", plugify::ValueType::ArrayInt16,
		"int32*", plugify::ValueType::ArrayInt32,
		"int64*", plugify::ValueType::ArrayInt64,
		"uint8*", plugify::ValueType::ArrayUint8,
		"uint16*", plugify::ValueType::ArrayUint16,
		"uint32*", plugify::ValueType::ArrayUint32,
		"uint64*", plugify::ValueType::ArrayUint64,
		"ptr64*", plugify::ValueType::ArrayPtr64,
		"float*", plugify::ValueType::ArrayFloat,
		"double*", plugify::ValueType::ArrayDouble,
		"string*", plugify::ValueType::ArrayString
    );
};

template<>
struct glz::meta<plugify::Severity> {
    static constexpr auto value = enumerate(
        "none", plugify::Severity::None,
        "fatal", plugify::Severity::Fatal,
        "error", plugify::Severity::Error,
        "warning", plugify::Severity::Warning,
        "info", plugify::Severity::Info,
        "debug", plugify::Severity::Debug,
        "verbose", plugify::Severity::Verbose
    );
};

GLZ_META(plugify::PluginDescriptor, fileVersion, version, versionName, friendlyName, description, createdBy, createdByURL, docsURL, downloadURL, updateURL, supportedPlatforms, assemblyPath, languageModule, dependencies, exportedMethods);
GLZ_META(plugify::LanguageModuleDescriptor, fileVersion, version, versionName, friendlyName, description, createdBy, createdByURL, docsURL, downloadURL, updateURL, supportedPlatforms, language, libraryDirectories, forceLoad);
GLZ_META(plugify::Config, baseDir, logSeverity, repositories);

GLZ_META(plugify::PackageVersion, version, mirrors, platforms);
GLZ_META(plugify::RemotePackage, name, type, author, description, versions);
GLZ_META(plugify::LocalPackage, name, type, path, version, descriptor);

template <>
struct glz::meta<plugify::Property> {
	using T = plugify::Property;
	static constexpr auto value = object("type", &T::type, "name", skip{}, "ref", &T::ref, "prototype", &T::prototype);
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