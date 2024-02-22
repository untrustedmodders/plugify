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
		"uint8", plugify::ValueType::UInt8,
		"uint16", plugify::ValueType::UInt16,
		"uint32", plugify::ValueType::UInt32,
		"uint64", plugify::ValueType::UInt64,
		"ptr64", plugify::ValueType::Ptr64,
		"float", plugify::ValueType::Float,
		"double", plugify::ValueType::Double,
		"function", plugify::ValueType::Function,
		"string", plugify::ValueType::String,
		"bool*", plugify::ValueType::ArrayBool,
		"char8*", plugify::ValueType::ArrayChar8,
		"char16*", plugify::ValueType::ArrayChar16,
		"int8*", plugify::ValueType::ArrayInt8,
		"int16*", plugify::ValueType::ArrayInt16,
		"int32*", plugify::ValueType::ArrayInt32,
		"int64*", plugify::ValueType::ArrayInt64,
		"uint8*", plugify::ValueType::ArrayUInt8,
		"uint16*", plugify::ValueType::ArrayUInt16,
		"uint32*", plugify::ValueType::ArrayUInt32,
		"uint64*", plugify::ValueType::ArrayUInt64,
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

template <>
struct glz::meta<plugify::PluginDescriptor> {
	using T = plugify::PluginDescriptor;
	static constexpr auto value = object(
		"fileVersion", &T::fileVersion,
		"version", &T::version,
		"versionName", &T::versionName,
		"friendlyName", &T::friendlyName,
		"description", &T::description,
		"createdBy", &T::createdBy,
		"createdByURL", &T::createdByURL,
		"docsURL", &T::docsURL,
		"downloadURL", &T::downloadURL,
		"updateURL", &T::updateURL,
		"supportedPlatforms", &T::supportedPlatforms,
		"entryPoint", &T::entryPoint,
		"languageModule", &T::languageModule,
		"dependencies", &T::dependencies,
		"exportedMethods", &T::exportedMethods
	);
};

template <>
struct glz::meta<plugify::LanguageModuleDescriptor> {
	using T = plugify::LanguageModuleDescriptor;
	static constexpr auto value = object(
		"fileVersion", &T::fileVersion,
		"version", &T::version,
		"versionName", &T::versionName,
		"friendlyName", &T::friendlyName,
		"description", &T::description,
		"createdBy", &T::createdBy,
		"createdByURL", &T::createdByURL,
		"docsURL", &T::docsURL,
		"downloadURL", &T::downloadURL,
		"updateURL", &T::updateURL,
		"supportedPlatforms", &T::supportedPlatforms,
		"language", &T::language,
		"libraryDirectories", &T::libraryDirectories,
		"forceLoad", &T::forceLoad
	);
};

template <>
struct glz::meta<plugify::Config> {
	using T = plugify::Config;
	static constexpr auto value = object(
		"baseDir", &T::baseDir,
		"logSeverity", &T::logSeverity,
		"repositories", &T::repositories
	);
};

template <>
struct glz::meta<plugify::PackageVersion> {
	using T = plugify::PackageVersion;
	static constexpr auto value = object(
		"version", &T::version,
		"checksum", &T::checksum,
		"download", &T::download,
		"platforms", &T::platforms
	);
};

template <>
struct glz::meta<plugify::RemotePackage> {
	using T = plugify::RemotePackage;
	static constexpr auto value = object(
		"name", &T::name,
		"type", &T::type,
		"author", &T::author,
		"description", &T::description,
		"versions", &T::versions
	);
};

template <>
struct glz::meta<plugify::LocalPackage> {
	using T = plugify::LocalPackage;
	static constexpr auto value = object(
		"name", &T::name,
		"type", &T::type,
		"path", &T::path,
		"version", &T::version,
		"descriptor", &T::descriptor
	);
};

template <>
struct glz::meta<plugify::Property> {
	using T = plugify::Property;
	static constexpr auto value = object(
		"type", &T::type,
		"name", skip{},
		"ref", &T::ref,
		"prototype", &T::prototype
	);
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