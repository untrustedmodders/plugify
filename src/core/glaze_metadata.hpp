#pragma once

#include <glaze/glaze.hpp>

#include "plugify/config.hpp"
#include "plugify/logger.hpp"
#include "plugify/manifest.hpp"

#include "core/conflict_impl.hpp"
#include "core/dependency_impl.hpp"
#include "core/enum_object_impl.hpp"
#include "core/enum_value_impl.hpp"
#include "core/method_impl.hpp"
#include "core/property_impl.hpp"

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
struct glz::meta<plugify::Manifest> {
    using T = plugify::Manifest;
    static constexpr auto value = object(
            "$schema", skip{},
            "metadata", skip{},

            "name", &T::name,
            "version", &T::version,
            "description", &T::description,
            "author", &T::author,
            "website", &T::website,
            "license", &T::license,

            "platforms", &T::platforms,
            "dependencies", &T::dependencies,
            "conflicts", &T::conflicts,
            "obsoletes", &T::obsoletes,
            "language", &T::language,

            "entry", &T::entry,
            "methods", &T::methods,
            "runtime", &T::runtime,
            "directories", &T::directories
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
struct glz::meta<plugify::Conflict> {
    static constexpr auto value = object(
        "name", [](auto&& self) -> auto& { return self._impl->name; },
        "constraints", [](auto&& self) -> auto& { return self._impl->constraints; },
        "reason", [](auto&& self) -> auto& { return self._impl->reason; }
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

template <>
struct glz::meta<plugify::EnumObject> {
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
	using N = plugify::ValueName;
	using T = plugify::ValueType;
	static constexpr auto value = enumerate(
			N::Void, T::Void,
			N::Bool, T::Bool,
			N::Char8, T::Char8,
			N::Char16, T::Char16,
			N::Int8, T::Int8,
			N::Int16, T::Int16,
			N::Int32, T::Int32,
			N::Int64, T::Int64,
			N::UInt8, T::UInt8,
			N::UInt16, T::UInt16,
			N::UInt32, T::UInt32,
			N::UInt64, T::UInt64,
			N::Pointer, T::Pointer,
			N::Float, T::Float,
			N::Double, T::Double,
			N::Function, T::Function,
			N::String, T::String,
			N::Any, T::Any,
			N::ArrayBool, T::ArrayBool,
			N::ArrayChar8, T::ArrayChar8,
			N::ArrayChar16, T::ArrayChar16,
			N::ArrayInt8, T::ArrayInt8,
			N::ArrayInt16, T::ArrayInt16,
			N::ArrayInt32, T::ArrayInt32,
			N::ArrayInt64, T::ArrayInt64,
			N::ArrayUInt8, T::ArrayUInt8,
			N::ArrayUInt16, T::ArrayUInt16,
			N::ArrayUInt32, T::ArrayUInt32,
			N::ArrayUInt64, T::ArrayUInt64,
			N::ArrayPointer, T::ArrayPointer,
			N::ArrayFloat, T::ArrayFloat,
			N::ArrayDouble, T::ArrayDouble,
			N::ArrayString, T::ArrayString,
			N::ArrayAny, T::ArrayAny,
			N::ArrayVector2, T::ArrayVector2,
			N::ArrayVector3, T::ArrayVector3,
			N::ArrayVector4, T::ArrayVector4,
			N::ArrayMatrix4x4, T::ArrayMatrix4x4,
			N::Vector2, T::Vector2,
			N::Vector3, T::Vector3,
			N::Vector4, T::Vector4,
			N::Matrix4x4, T::Matrix4x4
	);
};

namespace glz {
#if PLUGIFY_CPP_VERSION > 202002L
    // std::chrono::duration
    template <class R, class P>
    struct from<JSON, std::chrono::duration<R, P>> {
        template <auto Opts>
        static void op(auto&& value, auto&&... args) {
            R rep;
            parse<JSON>::op<Opts>(rep, args...);
            value = std::chrono::duration<R, P>(rep);
        }
    };
    template <class R, class P>
    struct to<JSON, std::chrono::duration<R, P>> {
        template <auto Opts>
        static void op(auto&& value, auto&&... args) noexcept {
            serialize<JSON>::op<Opts>(value.count(), args...);
        }
    };

    // std::chrono::time_point
    template <class C, class D>
    struct from<JSON, std::chrono::time_point<C, D>> {
        template <auto Opts>
        static void op(auto&& value, auto&&... args) {
            D duration;
            parse<JSON>::op<Opts>(duration, args...);
            value = std::chrono::time_point<C, D>(duration);
        }
    };
    template <class C, class D>
    struct to<JSON, std::chrono::time_point<C, D>> {
        template <auto Opts>
        static void op(auto&& value, auto&&... args) noexcept {
            serialize<JSON>::op<Opts>(value.time_since_epoch(), args...);
        }
    };

    // std::filesystem::path
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

    // plg::version
	template<>
	struct from<JSON, plugify::Version> {
		template <auto Opts>
		static void op(plugify::Version& value, auto&&... args) {
			std::string str;
			parse<JSON>::op<Opts>(str, args...);
            plg::parse(str, value);
        }
	};

	template<>
	struct to<JSON, plugify::Version> {
		template <auto Opts>
		static void op(const plugify::Version& value, auto&&... args) noexcept {
			serialize<JSON>::op<Opts>(value.to_string(), args...);
		}
	};

    // plg::range
	template<>
	struct from<JSON, plugify::Constraint> {
	    template <auto Opts>
	    static void op(plugify::Constraint& value, auto&&... args) {
            std::string str;
            parse<JSON>::op<Opts>(str, args...);
            plg::parse(str, value);
	    }
	};

	/*template<>
	struct to<JSON, plugify::Constraint> {
	    template <auto Opts>
	    static void op(const plugify::Constraint& value, auto&&... args) noexcept {
	    }
	};*/
#else
	namespace detail {
	    // std::chrono::duration
	    template <class R, class P>
        struct from_json<std::chrono::duration<R, P>> {
	        template <auto Opts>
            static void op(auto&& value, auto&&... args) {
	            R rep;
	            parse<JSON>::op<Opts>(rep, args...);
	            value = std::chrono::duration<R, P>(rep);
	        }
	    };
	    template <class R, class P>
        struct from_json<std::chrono::duration<R, P>> {
	        template <auto Opts>
            static void op(auto&& value, auto&&... args) noexcept {
	            serialize<JSON>::op<Opts>(value.count(), args...);
	        }
	    };

	    // std::chrono::time_point
	    template <class C, class D>
        struct from_json<std::chrono::time_point<C, D>> {
	        template <auto Opts>
            static void op(auto&& value, auto&&... args) {
	            D duration;
	            parse<JSON>::op<Opts>(duration, args...);
	            value = std::chrono::time_point<C, D>(duration);
	        }
	    };
	    template <class C, class D>
        struct from_json<std::chrono::time_point<C, D>> {
	        template <auto Opts>
            static void op(auto&& value, auto&&... args) noexcept {
	            serialize<JSON>::op<Opts>(value.time_since_epoch(), args...);
	        }
	    };

	    // std::filesystem::path
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
		        std::string str;
		        parse<JSON>::op<Opts>(str, args...);
                plg::parse(str, value);
		    }
		};

		/*template<>
		struct to_json<plugify::Constraint> {
		    template <auto Opts>
		    static void op(const plugify::Constraint& value, auto&&... args) noexcept {
		    }
		};*/
	}
#endif
}
