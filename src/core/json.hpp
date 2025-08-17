#pragma once

#include "manifest.hpp"
#include "method.hpp"
#include "module_manifest.hpp"
#include "plugin_manifest.hpp"

#include <plugify/api/config.hpp>

#include <glaze/glaze.hpp>

using namespace plugify;

template<>
struct glz::meta<ValueType> {
	static constexpr auto value = enumerate(
			ValueName::Void, ValueType::Void,
			ValueName::Bool, ValueType::Bool,
			ValueName::Char8, ValueType::Char8,
			ValueName::Char16, ValueType::Char16,
			ValueName::Int8, ValueType::Int8,
			ValueName::Int16, ValueType::Int16,
			ValueName::Int32, ValueType::Int32,
			ValueName::Int64, ValueType::Int64,
			ValueName::UInt8, ValueType::UInt8,
			ValueName::UInt16, ValueType::UInt16,
			ValueName::UInt32, ValueType::UInt32,
			ValueName::UInt64, ValueType::UInt64,
			ValueName::Pointer, ValueType::Pointer,
			ValueName::Float, ValueType::Float,
			ValueName::Double, ValueType::Double,
			ValueName::Function, ValueType::Function,
			ValueName::String, ValueType::String,
			ValueName::Any, ValueType::Any,
			ValueName::ArrayBool, ValueType::ArrayBool,
			ValueName::ArrayChar8, ValueType::ArrayChar8,
			ValueName::ArrayChar16, ValueType::ArrayChar16,
			ValueName::ArrayInt8, ValueType::ArrayInt8,
			ValueName::ArrayInt16, ValueType::ArrayInt16,
			ValueName::ArrayInt32, ValueType::ArrayInt32,
			ValueName::ArrayInt64, ValueType::ArrayInt64,
			ValueName::ArrayUInt8, ValueType::ArrayUInt8,
			ValueName::ArrayUInt16, ValueType::ArrayUInt16,
			ValueName::ArrayUInt32, ValueType::ArrayUInt32,
			ValueName::ArrayUInt64, ValueType::ArrayUInt64,
			ValueName::ArrayPointer, ValueType::ArrayPointer,
			ValueName::ArrayFloat, ValueType::ArrayFloat,
			ValueName::ArrayDouble, ValueType::ArrayDouble,
			ValueName::ArrayString, ValueType::ArrayString,
			ValueName::ArrayAny, ValueType::ArrayAny,
			ValueName::ArrayVector2, ValueType::ArrayVector2,
			ValueName::ArrayVector3, ValueType::ArrayVector3,
			ValueName::ArrayVector4, ValueType::ArrayVector4,
			ValueName::ArrayMatrix4x4, ValueType::ArrayMatrix4x4,
			ValueName::Vector2, ValueType::Vector2,
			ValueName::Vector3, ValueType::Vector3,
			ValueName::Vector4, ValueType::Vector4,
			ValueName::Matrix4x4, ValueType::Matrix4x4
	);
};

template<>
struct glz::meta<Severity> {
	static constexpr auto value = enumerate(
			"none", Severity::None,
			"fatal", Severity::Fatal,
			"error", Severity::Error,
			"warn", Severity::Warning,
			"info", Severity::Info,
			"debug", Severity::Debug,
			"verbose", Severity::Verbose
	);
};

template<>
struct glz::meta<ManifestType> {
	static constexpr auto value = enumerate(
			"module", ManifestType::Module,
			"plugin", ManifestType::Plugin
	);
};

template <>
struct glz::meta<Dependency> {
	using T = Dependency;
	static constexpr auto value = object(
			"name", &T::name,
			"constraints", &T::constraints,
			"optional", &T::optional
	);
};

template <>
struct glz::meta<Conflict> {
	using T = Conflict;
	static constexpr auto value = object(
			"name", &T::name,
			"constraints", &T::constraints,
			"reason", &T::reason
	);
};

template <>
struct glz::meta<Config> {
	using T = Config;
	static constexpr auto value = object(
			"$schema", skip{},
			"baseDir", &T::baseDir,
			"configsDir", &T::configsDir,
			"dataDir", &T::dataDir,
			"logsDir", &T::logsDir,
			"logSeverity", &T::logSeverity,
			"preferOwnSymbols", &T::preferOwnSymbols
	);
};

template <>
struct glz::meta<EnumValue> {
	using T = EnumValue;
	static constexpr auto value = object(
			"name", &T::name,
			"description", skip{},
			"value", &T::value
	);
};

template <>
struct glz::meta<Enum> {
	using T = Enum;
	static constexpr auto value = object(
			"name", &T::name,
			"description", skip{},
			"values", &T::values
	);
};

template <>
struct glz::meta<Property> {
	using T = Property;
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
struct glz::meta<Method> {
	using T = Method;
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
	struct from<JSON, Version> {
		template <auto Opts>
		static void op(Version& value, auto&&... args) {
			std::string str;
			parse<JSON>::op<Opts>(str, args...);
			value.from_string_noexcept(str);
		}
	};

	template<>
	struct to<JSON, Version> {
		template <auto Opts>
		static void op(const Version& value, auto&&... args) noexcept {
			serialize<JSON>::op<Opts>(value.to_string_noexcept(), args...);
		}
	};

	template<>
	struct from<JSON, Constraint> {
	    template <auto Opts>
	    static void op(Constraint& value, auto&&... args) {
	        std::string str;
	        parse<JSON>::op<Opts>(str, args...);

	        if (str.empty()) {
	            value.comparison = Comparison::Any;
	            value.version = Version{};
	            return;
	        }

	        // Parse the constraint operator
	        std::string_view sv(str);
	        Comparison comparison;
	        size_t op_len = 0;

	        if (sv.starts_with(">=")) {
	            comparison = Comparison::GreaterEqual;
	            op_len = 2;
	        } else if (sv.starts_with("<=")) {
	            comparison = Comparison::LessEqual;
	            op_len = 2;
	        } else if (sv.starts_with("~>")) {
	            comparison = Comparison::Compatible;
	            op_len = 2;
	        } else if (sv.starts_with("!=")) {
	            comparison = Comparison::NotEqual;
	            op_len = 2;
	        } else if (sv.starts_with("==")) {
	            comparison = Comparison::Equal;
	            op_len = 2;
	        } else if (sv.starts_with(">")) {
	            comparison = Comparison::Greater;
	            op_len = 1;
	        } else if (sv.starts_with("<")) {
	            comparison = Comparison::Less;
	            op_len = 1;
	        } else {
	            // No operator, assume equality
	            comparison = Comparison::Equal;
	            op_len = 0;
	        }

	        value.comparison = comparison;
	        value.version = Version(sv.substr(op_len));
	    }
	};

	template<>
	struct to<JSON, Constraint> {
	    template <auto Opts>
	    static void op(const Constraint& value, auto&&... args) noexcept {
	        std::string result;

	        // Convert constraint type to operator string
	        switch (value.comparison) {
	            case Comparison::Equal:
	                result = "==";
	                break;
	            case Comparison::NotEqual:
	                result = "!=";
	                break;
	            case Comparison::Greater:
	                result = ">";
	                break;
	            case Comparison::GreaterEqual:
	                result = ">=";
	                break;
	            case Comparison::Less:
	                result = "<";
	                break;
	            case Comparison::LessEqual:
	                result = "<=";
	                break;
	            case Comparison::Compatible:
	                result = "~>";
	                break;
	            case Comparison::Any:
	                serialize<JSON>::op<Opts>(result, args...);
	                return;
	        }

	        // Append version string
	        result += value.version.to_string_noexcept();

	        serialize<JSON>::op<Opts>(result, args...);
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
		struct from_json<Version> {
			template<auto Opts>
			static void op(Version& value, auto&&... args) {
				std::string str;
				read<json>::op<Opts>(str, args...);
				value.from_string_noexcept(str);
			}
		};

		template<>
		struct to_json<Version> {
			template<auto Opts>
			static void op(const Version& value, auto&&... args) noexcept {
				write<json>::op<Opts>(value.to_string_noexcept(), args...);
			}
		};

		template<>
		struct from_json<Constraint> {
		    template <auto Opts>
		    static void op(Constraint& value, auto&&... args) {
		        std::string str;
		        parse<JSON>::op<Opts>(str, args...);

		        if (str.empty()) {
		            value.type = Comparison::Any;
		            value.version = Version{};
		            return;
		        }

		        // Parse the constraint operator
		        std::string_view sv(str);
		        VersionConstraintType type;
		        size_t op_len = 0;

		        if (sv.starts_with(">=")) {
		            type = Comparison::GreaterEqual;
		            op_len = 2;
		        } else if (sv.starts_with("<=")) {
		            type = Comparison::LessEqual;
		            op_len = 2;
		        } else if (sv.starts_with("~>")) {
		            type = Comparison::Compatible;
		            op_len = 2;
		        } else if (sv.starts_with("!=")) {
		            type = Comparison::NotEqual;
		            op_len = 2;
		        } else if (sv.starts_with("==")) {
		            type = Comparison::Equal;
		            op_len = 2;
		        } else if (sv.starts_with(">")) {
		            type = Comparison::Greater;
		            op_len = 1;
		        } else if (sv.starts_with("<")) {
		            type = Comparison::Less;
		            op_len = 1;
		        } else {
		            // No operator, assume equality
		            type = Comparison::Equal;
		            op_len = 0;
		        }

		        value.type = type;
		        value.version = Version(sv.substr(op_len));
		    }
		};

		template<>
		struct to_json<Constraint> {
		    template <auto Opts>
		    static void op(const Constraint& value, auto&&... args) noexcept {
		        std::string result;

		        // Convert constraint type to operator string
		        switch (value.type) {
		            case Comparison::Equal:
		                result = "==";
		                break;
		            case Comparison::NotEqual:
		                result = "!=";
		                break;
		            case Comparison::Greater:
		                result = ">";
		                break;
		            case Comparison::GreaterEqual:
		                result = ">=";
		                break;
		            case Comparison::Less:
		                result = "<";
		                break;
		            case Comparison::LessEqual:
		                result = "<=";
		                break;
		            case Comparison::Compatible:
		                result = "~>";
		                break;
		            case Comparison::Any:
		                serialize<JSON>::op<Opts>(result, args...);
		                return;
		        }

		        // Append version string
		        result += value.version.to_string_noexcept();

		        serialize<JSON>::op<Opts>(result, args...);
		    }
		};
	}
#endif
}

template <>
struct glz::meta<ModuleManifest> {
	using T = ModuleManifest;
	static constexpr auto value = object(
			"$schema", skip{},
			"path", skip{},
			"type", skip{},

			"name", &T::name,
			"version", &T::version,
			"description", &T::description,
			"author", &T::author,
			"website", &T::website,
			"license", &T::license,
			"platforms", &T::platforms,
			"dependencies", &T::dependencies,
			"conflicts", &T::conflicts,

			"language", &T::language,
			"directories", &T::directories,
			"forceLoad", &T::forceLoad
	);
};

template <>
struct glz::meta<PluginManifest> {
	using T = PluginManifest;
	static constexpr auto value = object(
			"$schema", skip{},
			"path", skip{},
			"type", skip{},

			"name", &T::name,
			"version", &T::version,
			"description", &T::description,
			"author", &T::author,
			"website", &T::website,
			"license", &T::license,
			"platforms", &T::platforms,
			"dependencies", &T::dependencies,
			"conflicts", &T::conflicts,

			"entry", &T::entry,
			"language", &T::language,
			"methods", &T::methods
	);
};
