#pragma once

#include "plugify/core/config.hpp"
#include "plugify/core/constraint.hpp"
#include "plugify/core/enum.hpp"
#include "plugify/core/manifest.hpp"
#include "plugify/core/method.hpp"

#include <glaze/glaze.hpp>

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
			"preferOwnSymbols", &T::preferOwnSymbols
	);
};

template <>
struct glz::meta<plugify::Conflict> {
	static constexpr auto value = object(
		"name", [](auto&& self) -> auto& { return self._impl->name; },
		"constraints", [](auto&& self) -> auto& { return self._impl->constraints; },
		"reason", [](auto&& self) -> auto& { return self._impl->reason; }
	);
};

template<>
struct glz::meta<plugify::Severity> {
	using T = plugify::Severity;
	static constexpr auto value = enumerate(
			"none", T::None,
			"fatal", T::Fatal,
			"error", T::Error,
			"warn", T::Warning,
			"info", T::Info,
			"debug", T::Debug,
			"verbose", T::Verbose
	);
};

template <>
struct glz::meta<plugify::Method> {
	static constexpr auto value = object(
		"group", skip{},
		"description", skip{},
		"paramTypes", [](auto&& self) -> auto& { return self._impl->paramTypes; },
		"retType", [](auto&& self) -> auto& { return self._impl->retType; },
		"varIndex", [](auto&& self) -> auto& { return self._impl->varIndex; },
		"name", [](auto&& self) -> auto& { return self._impl->name; },
		"funcName", [](auto&& self) -> auto& { return self._impl->funcName; },
		"callConv", [](auto&& self) -> auto& { return self._impl->callConv; }
	);
};

template <>
struct glz::meta<plugify::ModuleManifest> {
	using T = plugify::ModuleManifest;
	static constexpr auto value = object(
			"$schema", skip{},
			"id", skip{},
			"type", skip{},
			"location", skip{},

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
			"runtime", &T::runtime,
			"directories", &T::directories,
			"forceLoad", &T::forceLoad
	);
};

template <>
struct glz::meta<plugify::PluginManifest> {
	using T = plugify::PluginManifest;
	static constexpr auto value = object(
			"$schema", skip{},
			"id", skip{},
			"type", skip{},
			"location", skip{},

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
			"entry", &T::entry,
			"capabilities", &T::capabilities,
			"methods", &T::methods
	);
};

template <>
struct glz::meta<plugify::Property> {
	static constexpr auto value = object(
		"name", skip{},
		"description", skip{},
		"type", [](auto&& self) -> auto& { return self._impl->type; },
		"ref", [](auto&& self) -> auto& { return self._impl->ref; },
		"prototype", [](auto&& self) -> auto& { return self._impl->prototype; },
		"enumerate", [](auto&& self) -> auto& { return self._impl->enumerate; },
		"default", skip{}
	);
};

template<>
struct glz::meta<plugify::Dependency> {
	static constexpr auto value = object(
		"name", [](auto&& self) -> auto& { return self._impl->name; },
		"constraints", [](auto&& self) -> auto& { return self._impl->constraints; },
		"optional", [](auto&& self) -> auto& { return self._impl->optional; }
	);
};

template <>
struct glz::meta<plugify::Enum> {
	static constexpr auto value = object(
		"name", [](auto&& self) -> auto& { return self._impl->name; },
		"description", skip{},
		"values", [](auto&& self) -> auto& { return self._impl->values; }
	);
};

template <>
struct glz::meta<plugify::EnumValue> {
	static constexpr auto value = object(
		"name", [](auto&& self) -> auto& { return self._impl->name; },
		"description", skip{},
		"value", [](auto&& self) -> auto& { return self._impl->value; }
	);
};

template<>
struct glz::meta<plugify::ValueType> {
	using T = plugify::ValueType;
	static constexpr auto value = enumerate(
			T::Void, T::Void,
			T::Bool, T::Bool,
			T::Char8, T::Char8,
			T::Char16, T::Char16,
			T::Int8, T::Int8,
			T::Int16, T::Int16,
			T::Int32, T::Int32,
			T::Int64, T::Int64,
			T::UInt8, T::UInt8,
			T::UInt16, T::UInt16,
			T::UInt32, T::UInt32,
			T::UInt64, T::UInt64,
			T::Pointer, T::Pointer,
			T::Float, T::Float,
			T::Double, T::Double,
			T::Function, T::Function,
			T::String, T::String,
			T::Any, T::Any,
			T::ArrayBool, T::ArrayBool,
			T::ArrayChar8, T::ArrayChar8,
			T::ArrayChar16, T::ArrayChar16,
			T::ArrayInt8, T::ArrayInt8,
			T::ArrayInt16, T::ArrayInt16,
			T::ArrayInt32, T::ArrayInt32,
			T::ArrayInt64, T::ArrayInt64,
			T::ArrayUInt8, T::ArrayUInt8,
			T::ArrayUInt16, T::ArrayUInt16,
			T::ArrayUInt32, T::ArrayUInt32,
			T::ArrayUInt64, T::ArrayUInt64,
			T::ArrayPointer, T::ArrayPointer,
			T::ArrayFloat, T::ArrayFloat,
			T::ArrayDouble, T::ArrayDouble,
			T::ArrayString, T::ArrayString,
			T::ArrayAny, T::ArrayAny,
			T::ArrayVector2, T::ArrayVector2,
			T::ArrayVector3, T::ArrayVector3,
			T::ArrayVector4, T::ArrayVector4,
			T::ArrayMatrix4x4, T::ArrayMatrix4x4,
			T::Vector2, T::Vector2,
			T::Vector3, T::Vector3,
			T::Vector4, T::Vector4,
			T::Matrix4x4, T::Matrix4x4
	);
};

namespace glz {
#if PLUGIFY_CPP_VERSION > 202002L
	template<>
	struct from<JSON, std::filesystem::path> {
		template <auto Opts>
		static void op(std::filesystem::path& value, auto&&... args) {
			std::string str;
			parse<JSON>::op<Opts>(str, args...);
			value = str;
			if (!value.empty()) value.make_preferred();
		}
	};

	template<>
	struct to<JSON, std::filesystem::path> {
		template <auto Opts>
		static void op(const std::filesystem::path& value, auto&&... args) noexcept {
			serialize<JSON>::op<Opts>(value.generic_string(), args...);
		}
	};

	template<>
	struct from<JSON, plugify::Version> {
		template <auto Opts>
		static void op(plugify::Version& value, auto&&... args) {
			std::string str;
			parse<JSON>::op<Opts>(str, args...);
			value.from_string_noexcept(str);
		}
	};

	template<>
	struct to<JSON, plugify::Version> {
		template <auto Opts>
		static void op(const plugify::Version& value, auto&&... args) noexcept {
			serialize<JSON>::op<Opts>(value.to_string_noexcept(), args...);
		}
	};

	template<>
	struct from<JSON, plugify::Constraint> {
	    template <auto Opts>
	    static void op(plugify::Constraint& value, auto&&... args) {
		    using namespace plugify;
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
	struct to<JSON, plugify::Constraint> {
	    template <auto Opts>
	    static void op(const plugify::Constraint& value, auto&&... args) noexcept {
		    using namespace plugify;
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
		struct from_json<std::filesystem::path> {
			template<auto Opts>
			static void op(std::filesystem::path& value, auto&&... args) {
				std::string str;
				read<json>::op<Opts>(str, args...);
				value = str;
				if (!value.empty()) value.make_preferred();
			}
		};

		template<>
		struct to_json<std::filesystem::path> {
			template<auto Opts>
			static void op(const std::filesystem::path& value, auto&&... args) noexcept {
				write<json>::op<Opts>(value.generic_string(), args...);
			}
		};

		template<>
		struct from_json<plugify::Version> {
			template<auto Opts>
			static void op(plugify::Version& value, auto&&... args) {
				std::string str;
				read<json>::op<Opts>(str, args...);
				value.from_string_noexcept(str);
			}
		};

		template<>
		struct to_json<plugify::Version> {
			template<auto Opts>
			static void op(const plugify::Version& value, auto&&... args) noexcept {
				write<json>::op<Opts>(value.to_string_noexcept(), args...);
			}
		};

		template<>
		struct from_json<plugify::Constraint> {
		    template <auto Opts>
		    static void op(plugify::Constraint& value, auto&&... args) {
		    	using namespace plugify;
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
		struct to_json<plugify::Constraint> {
		    template <auto Opts>
		    static void op(const plugify::Constraint& value, auto&&... args) noexcept {
		    	using namespace plugify;
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
