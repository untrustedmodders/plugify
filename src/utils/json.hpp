#pragma once

#include <glaze/glaze.hpp>

#include <core/language_module_descriptor.hpp>
#include <core/method.hpp>
#include <core/plugin_descriptor.hpp>
#include <plugify/config.hpp>
#include <plugify/descriptor.hpp>
#include <plugify/package.hpp>

template<>
struct glz::meta<plugify::ValueType> {
	static constexpr auto value = enumerate(
			plugify::ValueName::Void, plugify::ValueType::Void,
			plugify::ValueName::Bool, plugify::ValueType::Bool,
			plugify::ValueName::Char8, plugify::ValueType::Char8,
			plugify::ValueName::Char16, plugify::ValueType::Char16,
			plugify::ValueName::Int8, plugify::ValueType::Int8,
			plugify::ValueName::Int16, plugify::ValueType::Int16,
			plugify::ValueName::Int32, plugify::ValueType::Int32,
			plugify::ValueName::Int64, plugify::ValueType::Int64,
			plugify::ValueName::UInt8, plugify::ValueType::UInt8,
			plugify::ValueName::UInt16, plugify::ValueType::UInt16,
			plugify::ValueName::UInt32, plugify::ValueType::UInt32,
			plugify::ValueName::UInt64, plugify::ValueType::UInt64,
			plugify::ValueName::Pointer, plugify::ValueType::Pointer,
			plugify::ValueName::Float, plugify::ValueType::Float,
			plugify::ValueName::Double, plugify::ValueType::Double,
			plugify::ValueName::Function, plugify::ValueType::Function,
			plugify::ValueName::String, plugify::ValueType::String,
			plugify::ValueName::Any, plugify::ValueType::Any,
			plugify::ValueName::ArrayBool, plugify::ValueType::ArrayBool,
			plugify::ValueName::ArrayChar8, plugify::ValueType::ArrayChar8,
			plugify::ValueName::ArrayChar16, plugify::ValueType::ArrayChar16,
			plugify::ValueName::ArrayInt8, plugify::ValueType::ArrayInt8,
			plugify::ValueName::ArrayInt16, plugify::ValueType::ArrayInt16,
			plugify::ValueName::ArrayInt32, plugify::ValueType::ArrayInt32,
			plugify::ValueName::ArrayInt64, plugify::ValueType::ArrayInt64,
			plugify::ValueName::ArrayUInt8, plugify::ValueType::ArrayUInt8,
			plugify::ValueName::ArrayUInt16, plugify::ValueType::ArrayUInt16,
			plugify::ValueName::ArrayUInt32, plugify::ValueType::ArrayUInt32,
			plugify::ValueName::ArrayUInt64, plugify::ValueType::ArrayUInt64,
			plugify::ValueName::ArrayPointer, plugify::ValueType::ArrayPointer,
			plugify::ValueName::ArrayFloat, plugify::ValueType::ArrayFloat,
			plugify::ValueName::ArrayDouble, plugify::ValueType::ArrayDouble,
			plugify::ValueName::ArrayString, plugify::ValueType::ArrayString,
			plugify::ValueName::ArrayAny, plugify::ValueType::ArrayAny,
			plugify::ValueName::ArrayVector2, plugify::ValueType::ArrayVector2,
			plugify::ValueName::ArrayVector3, plugify::ValueType::ArrayVector3,
			plugify::ValueName::ArrayVector4, plugify::ValueType::ArrayVector4,
			plugify::ValueName::ArrayMatrix4x4, plugify::ValueType::ArrayMatrix4x4,
			plugify::ValueName::Vector2, plugify::ValueType::Vector2,
			plugify::ValueName::Vector3, plugify::ValueType::Vector3,
			plugify::ValueName::Vector4, plugify::ValueType::Vector4,
			plugify::ValueName::Matrix4x4, plugify::ValueType::Matrix4x4
	);
};

template<>
struct glz::meta<plugify::Severity> {
	static constexpr auto value = enumerate(
			"none", plugify::Severity::None,
			"fatal", plugify::Severity::Fatal,
			"error", plugify::Severity::Error,
			"warn", plugify::Severity::Warning,
			"info", plugify::Severity::Info,
			"debug", plugify::Severity::Debug,
			"verbose", plugify::Severity::Verbose
	);
};

template <>
struct glz::meta<plugify::PluginDescriptor> {
	using T = plugify::PluginDescriptor;
	static constexpr auto value = object(
			"$schema", skip{},
			"fileVersion", &T::fileVersion,
			"version", &T::version,
			"versionName", skip{},
			"friendlyName", &T::friendlyName,
			"description", &T::description,
			"createdBy", &T::createdBy,
			"createdByURL", &T::createdByURL,
			"docsURL", &T::docsURL,
			"downloadURL", &T::downloadURL,
			"updateURL", &T::updateURL,
			"supportedPlatforms", &T::supportedPlatforms,
			"resourceDirectories", skip{}, // deprecated
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
			"$schema", skip{},
			"fileVersion", &T::fileVersion,
			"version", &T::version,
			"versionName", skip{},
			"friendlyName", &T::friendlyName,
			"description", &T::description,
			"createdBy", &T::createdBy,
			"createdByURL", &T::createdByURL,
			"docsURL", &T::docsURL,
			"downloadURL", &T::downloadURL,
			"updateURL", &T::updateURL,
			"supportedPlatforms", &T::supportedPlatforms,
			"resourceDirectories", skip{}, // deprecated
			"language", &T::language,
			"libraryDirectories", &T::libraryDirectories,
			"forceLoad", &T::forceLoad
	);
};

template <>
struct glz::meta<plugify::PluginReferenceDescriptor> {
	using T = plugify::PluginReferenceDescriptor;
	static constexpr auto value = object(
			"name", &T::name,
			"optional", &T::optional,
			"supportedPlatforms", &T::supportedPlatforms,
			"requestedVersion", &T::requestedVersion
	);
};

template <>
struct glz::meta<plugify::Config> {
	using T = plugify::Config;
	static constexpr auto value = object(
			"$schema", skip{},
			"baseDir", &T::baseDir,
			"configsDir", &T::configsDir,
			"dataDir", &T::dataDir,
			"logsDir", &T::logsDir,
			"logSeverity", &T::logSeverity,
			"repositories", &T::repositories,
			"preferOwnSymbols", &T::preferOwnSymbols
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
			"$schema", skip{},
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
			"$schema", skip{},
			"name", &T::name,
			"type", &T::type,
			"path", &T::path,
			"version", &T::version,
			"descriptor", &T::descriptor
	);
};

template <>
struct glz::meta<plugify::PackageManifest> {
	using T = plugify::PackageManifest;
	static constexpr auto value = object(
			"$schema", skip{},
			"content", &T::content
	);
};

template <>
struct glz::meta<plugify::EnumValue> {
	using T = plugify::EnumValue;
	static constexpr auto value = object(
			"name", &T::name,
			"description", skip{},
			"value", &T::value
	);
};


template <>
struct glz::meta<plugify::Enum> {
	using T = plugify::Enum;
	static constexpr auto value = object(
			"name", &T::name,
			"description", skip{},
			"values", &T::values
	);
};

template <>
struct glz::meta<plugify::Property> {
	using T = plugify::Property;
	static constexpr auto value = object(
			"type", &T::type,
			"name", skip{},
			"description", skip{},
			"ref", &T::ref,
			"prototype", &T::prototype,
			"enum", &T::enumerate,
			"default", skip{}
	);
};

template <>
struct glz::meta<plugify::Method> {
	using T = plugify::Method;
	static constexpr auto value = object(
			"name", &T::name,
			"group", skip{},
			"description", skip{},
			"funcName", &T::funcName,
			"callConv", &T::callConv,
			"paramTypes", &T::paramTypes,
			"retType", &T::retType,
			"varIndex", &T::varIndex
	);
};

namespace glz {
#if PLUGIFY_CPP_VERSION > 202002L
	template<>
	struct from<JSON, fs::path> {
		template <auto Opts>
		static void op(fs::path& value, auto&&... args) {
			std::string str;
			parse<JSON>::op<Opts>(str, args...);
			value = str;
			if (!value.empty()) value.make_preferred();
		}
	};

	template<>
	struct to<JSON, fs::path> {
		template <auto Opts>
		static void op(const fs::path& value, auto&&... args) noexcept {
			serialize<JSON>::op<Opts>(value.generic_string(), args...);
		}
	};

	template<>
	struct from<JSON, plg::version> {
		template <auto Opts>
		static void op(plg::version& value, auto&&... args) {
			std::string str;
			parse<JSON>::op<Opts>(str, args...);
			value.from_string_noexcept(str);
		}
	};

	template<>
	struct to<JSON, plg::version> {
		template <auto Opts>
		static void op(const plg::version& value, auto&&... args) noexcept {
			serialize<JSON>::op<Opts>(value.to_string_noexcept(), args...);
		}
	};
#else
	namespace detail {
		template<>
		struct from_json<fs::path> {
			template<auto Opts>
			static void op(fs::path& value, auto&&... args) {
				std::string str;
				read<json>::op<Opts>(str, args...);
				value = str;
				if (!value.empty()) value.make_preferred();
			}
		};

		template<>
		struct to_json<fs::path> {
			template<auto Opts>
			static void op(const fs::path& value, auto&&... args) noexcept {
				write<json>::op<Opts>(value.generic_string(), args...);
			}
		};

		template<>
		struct from_json<plg::version> {
			template<auto Opts>
			static void op(plg::version& value, auto&&... args) {
				std::string str;
				read<json>::op<Opts>(str, args...);
				value.from_string_noexcept(str);
			}
		};

		template<>
		struct to_json<plg::version> {
			template<auto Opts>
			static void op(const plg::version& value, auto&&... args) noexcept {
				write<json>::op<Opts>(value.to_string_noexcept(), args...);
			}
		};
	}
#endif
}
