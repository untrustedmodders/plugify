#pragma once

#include <glaze/glaze.hpp>
#include <glaze/core/macros.hpp>

#include <wizard/method.h>
#include <wizard/descriptor.h>
#include <wizard/plugin_descriptor.h>
#include <wizard/language_module_descriptor.h>
#include <wizard/config.h>
#include <wizard/package.h>

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
        "function", wizard::ValueType::Function,
		"bool*", wizard::ValueType::ArrayBool,
		"char8*", wizard::ValueType::ArrayChar8,
		"char16*", wizard::ValueType::ArrayChar16,
		"int8*", wizard::ValueType::ArrayInt8,
		"int16*", wizard::ValueType::ArrayInt16,
		"int32*", wizard::ValueType::ArrayInt32,
		"int64*", wizard::ValueType::ArrayInt64,
		"uint8*", wizard::ValueType::ArrayUint8,
		"uint16*", wizard::ValueType::ArrayUint16,
		"uint32*", wizard::ValueType::ArrayUint32,
		"uint64*", wizard::ValueType::ArrayUint64,
		"ptr64*", wizard::ValueType::ArrayPtr64,
		"float*", wizard::ValueType::ArrayFloat,
		"double*", wizard::ValueType::ArrayDouble,
		"string*", wizard::ValueType::ArrayString
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

GLZ_META(wizard::PluginDescriptor, fileVersion, version, versionName, friendlyName, description, createdBy, createdByURL, docsURL, downloadURL, updateURL, supportedPlatforms, assemblyPath, languageModule, dependencies, exportedMethods);
GLZ_META(wizard::LanguageModuleDescriptor, fileVersion, version, versionName, friendlyName, description, createdBy, createdByURL, docsURL, downloadURL, updateURL, supportedPlatforms, language, forceLoad);
GLZ_META(wizard::Config, baseDir, logSeverity, repositories);

GLZ_META(wizard::PackageVersion, version, mirrors, platforms);
GLZ_META(wizard::RemotePackage, name, type, author, description, versions);
GLZ_META(wizard::LocalPackage, name, type, path, version, descriptor);

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