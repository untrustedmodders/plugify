#include "plugify/manifest.hpp"

#include "core/binding_impl.hpp"
#include "core/class_impl.hpp"
#include "core/enum_object_impl.hpp"
#include "core/enum_value_impl.hpp"
#include "core/method_impl.hpp"
#include "core/property_impl.hpp"

using namespace plugify;

namespace {
	// A manifest reaches Validate() only after ParsingStage has checked it against
	// the JSON schema for its extension type, so every rule a schema can state -
	// which fields must be present, what type each holds, and how each one is
	// spelled - has already been enforced, with a better error message than
	// anything reachable from here. Repeating those rules would only let the two
	// descriptions drift apart.
	//
	// What is left is what JSON Schema cannot say: names that must be unique
	// across an array, fields that constrain one another, and the invariants
	// Resolve() is expected to have established.

	// The schema guarantees a non-empty list of well-named values. It cannot
	// guarantee that no two of them share a name, which would make the generated
	// binding ambiguous. Numbers are deliberately left alone: flag enums and
	// aliases legitimately repeat them.
	Result<void> ValidateEnumObject(const EnumObject::Impl& enumObj) {
		std::unordered_set<std::string_view> valueNames;
		valueNames.reserve(enumObj.values.size());

		for (const auto& v : enumObj.values) {
			const auto& value = *v._impl;

			if (!valueNames.insert(value.name).second) {
				return MakeError("Duplicate enum value name: {} in enum '{}'", value.name, enumObj.name);
			}
		}

		return {};
	}

	// The prototype and enum a property points at are validated once via the
	// manifest's shared tables, which Resolve() has already populated with every
	// definition; recursing into them here would both repeat that work and hang
	// on prototypes that refer to one another.
	Result<void> ValidateProperty(const Property::Impl& prop, std::string_view context, bool param) {
		// The schema fixes the set of type names, but not where each one may be
		// used: nothing can be passed as void.
		if (param && prop.type == ValueType::Void) {
			return MakeError("{}: Parameter cannot be void type", context);
		}

		// Still holding a name means Resolve() never ran, or ran before this
		// property was added; either way nothing downstream can use the property.
		if (const auto* reference = std::get_if<std::string>(&prop.prototype)) {
			return MakeError("{}: Unresolved prototype reference '{}'", context, *reference);
		}

		if (const auto* reference = std::get_if<std::string>(&prop.enumerate)) {
			return MakeError("{}: Unresolved enum reference '{}'", context, *reference);
		}

		// A callback with no signature cannot be marshalled in either direction.
		if (prop.type == ValueType::Function && !DefinitionOf(prop.prototype)) {
			return MakeError("{}: Function type requires prototype", context);
		}

		return {};
	}

	// The schema checks each parameter and the return value on its own; it cannot
	// see how they relate to the method holding them. `prefix` names the kind of
	// signature being checked, for the error message.
	Result<void> ValidateMethod(const Method::Impl& method, std::string_view prefix) {
		// An absent varIndex and the explicit kNoVarArgs sentinel both mean "not
		// variadic"; any other value has to name a parameter that exists.
		if (method.varIndex && *method.varIndex != Signature::kNoVarArgs
			&& *method.varIndex >= method.paramTypes.size()) {
			return MakeError(
				"{} '{}': varIndex {} is out of range, the method takes {} parameter(s)",
				prefix,
				method.name,
				*method.varIndex,
				method.paramTypes.size()
			);
		}

		for (size_t i = 0; i < method.paramTypes.size(); ++i) {
			if (auto result = ValidateProperty(
					*method.paramTypes[i]._impl,
					std::format("{} '{}' param[{}]", prefix, method.name, i),
					true
				);
				!result) {
				return result;
			}
		}

		return ValidateProperty(
			*method.retType._impl,
			std::format("{} '{}' return type", prefix, method.name),
			false
		);
	}

	// Validate Class
	Result<void> ValidateClass(const Class::Impl& classObj) {
		// A class with no handle is a namespace for free functions: there is no
		// instance to construct, to destroy, or to pass as an implicit argument.
		const bool isHandleless = classObj.handleType.value_or(ValueType::Void) == ValueType::Void;

		if (isHandleless && (classObj.constructors || classObj.destructor)) {
			return MakeError(
				"Class '{}': handleless classes cannot have constructors or destructors",
				classObj.name
			);
		}

		std::unordered_set<std::string_view> bindingNames;
		bindingNames.reserve(classObj.bindings.size());

		for (const auto& binding : classObj.bindings) {
			const auto& bind = *binding._impl;

			if (!bindingNames.insert(bind.name).second) {
				return MakeError("Class '{}': duplicate binding name '{}'", classObj.name, bind.name);
			}

			if (isHandleless && bind.bindSelf.value_or(false)) {
				return MakeError(
					"Class '{}': binding '{}' sets bindSelf, but a handleless class has no instance to bind",
					classObj.name,
					bind.name
				);
			}
		}

		return {};
	}


	// Two definitions written under the same name are only allowed if they say the
	// same thing, so that repeating an inline enum across several methods keeps
	// working. Nested prototypes and enums compare by name rather than by value:
	// within a manifest a name denotes exactly one type, which also keeps this
	// terminating when prototypes refer to one another.
	template <class T>
	std::string_view DefinitionName(const Definition<T>& def) noexcept {
		if (const auto* reference = std::get_if<std::string>(&def)) {
			return *reference;
		}
		const auto& definition = std::get<std::shared_ptr<T>>(def);
		return definition ? definition->_impl->name : std::string_view{};
	}

	bool SameDefinition(const Property::Impl& lhs, const Property::Impl& rhs) {
		return lhs.type == rhs.type
			&& lhs.ref.value_or(false) == rhs.ref.value_or(false)
			&& DefinitionName(lhs.prototype) == DefinitionName(rhs.prototype)
			&& DefinitionName(lhs.enumerate) == DefinitionName(rhs.enumerate);
	}

	bool SameDefinition(const EnumObject::Impl& lhs, const EnumObject::Impl& rhs) {
		return std::ranges::equal(lhs.values, rhs.values, [](const EnumValue& a, const EnumValue& b) {
			return a._impl->name == b._impl->name && a._impl->value == b._impl->value;
		});
	}

	bool SameDefinition(const Method::Impl& lhs, const Method::Impl& rhs) {
		if (lhs.funcName != rhs.funcName || lhs.callConv != rhs.callConv
			|| lhs.varIndex != rhs.varIndex) {
			return false;
		}

		if (!SameDefinition(*lhs.retType._impl, *rhs.retType._impl)) {
			return false;
		}

		return std::ranges::equal(lhs.paramTypes, rhs.paramTypes, [](const Property& a, const Property& b) {
			return SameDefinition(*a._impl, *b._impl);
		});
	}

	// Gathers every prototype and enum in a manifest into one table keyed by name,
	// so a definition declared up front and the same definition written inline
	// collapse onto a single shared object.
	struct TypeTable {
		std::unordered_map<std::string, std::shared_ptr<Method>> prototypes;
		std::unordered_map<std::string, std::shared_ptr<EnumObject>> enums;

		// Returns whether this definition is the first one seen under its name; a
		// later duplicate is collapsed onto the first so that consumers can treat
		// pointer identity as type identity.
		// `definition` is null when the field is absent, and the pointer itself is
		// null when the field is still holding a name rather than a definition.
		template <class T>
		Result<bool> Register(
			std::unordered_map<std::string, std::shared_ptr<T>>& table,
			std::shared_ptr<T>* definition,
			std::string_view kind,
			std::string_view context
		) {
			if (!definition || !*definition) {
				return false;
			}

			const auto& name = (*definition)->_impl->name;
			if (name.empty()) {
				return MakeError("{}: {} definition must have a name", context, kind);
			}

			auto [it, inserted] = table.try_emplace(name, *definition);
			if (inserted) {
				return true;
			}

			if (it->second != *definition
				&& !SameDefinition(*it->second->_impl, *(*definition)->_impl)) {
				return MakeError("{}: conflicting definitions for {} '{}'", context, kind, name);
			}

			*definition = it->second;
			return false;
		}

		// Pass one: register the inline definitions hanging off this property, then
		// descend into an inline prototype's own parameters. Inline definitions nest
		// as a tree, so this always terminates.
		Result<void> Collect(Property::Impl& prop, std::string_view context) {
			auto* inline_prototype = std::get_if<std::shared_ptr<Method>>(&prop.prototype);
			auto prototype = Register(prototypes, inline_prototype, "prototype", context);
			if (!prototype) {
				return MakeError(std::move(prototype.error()));
			}

			auto* inline_enum = std::get_if<std::shared_ptr<EnumObject>>(&prop.enumerate);
			if (auto enumerate = Register(enums, inline_enum, "enum", context); !enumerate) {
				return MakeError(std::move(enumerate.error()));
			}

			// Only descend into a definition this call introduced; one that collapsed
			// onto an earlier definition has already been walked.
			if (*prototype) {
				return Collect(*(*inline_prototype)->_impl, context);
			}

			return {};
		}

		Result<void> Collect(Method::Impl& method, std::string_view context) {
			for (size_t i = 0; i < method.paramTypes.size(); ++i) {
				if (auto result = Collect(
						*method.paramTypes[i]._impl,
						std::format("{} '{}' param[{}]", context, method.name, i)
					);
					!result) {
					return result;
				}
			}

			return Collect(*method.retType._impl, std::format("{} '{}' return type", context, method.name));
		}

		// Pass two: swap each by-name reference for the definition it names. Every
		// definition is in the table by now, so references may point forwards, and
		// no recursion is needed.
		template <class T>
		Result<void> Link(
			const std::unordered_map<std::string, std::shared_ptr<T>>& table,
			Definition<T>& def,
			std::string_view kind,
			std::string_view context
		) {
			const auto* reference = std::get_if<std::string>(&def);
			if (!reference) {
				return {};
			}

			auto it = table.find(*reference);
			if (it == table.end()) {
				return MakeError("{}: unknown {} '{}'", context, kind, *reference);
			}

			// Assigning the definition destroys the name `reference` points at, so
			// nothing may read it past this line.
			def = it->second;
			return {};
		}

		Result<void> Link(Method::Impl& method, std::string_view context) {
			auto link = [&](Property::Impl& prop, std::string_view where) -> Result<void> {
				if (auto result = Link(prototypes, prop.prototype, "prototype", where); !result) {
					return result;
				}
				return Link(enums, prop.enumerate, "enum", where);
			};

			for (size_t i = 0; i < method.paramTypes.size(); ++i) {
				if (auto result = link(
						*method.paramTypes[i]._impl,
						std::format("{} '{}' param[{}]", context, method.name, i)
					);
					!result) {
					return result;
				}
			}

			return link(*method.retType._impl, std::format("{} '{}' return type", context, method.name));
		}

		// A prototype that can reach itself makes the shared_ptr graph
		// self-referential, so it would never be freed, and sends anything that
		// walks a signature recursively - Method::FindPrototype and every code
		// generator among them - into unbounded recursion. Names are still to hand
		// here, so reject it with one.
		Result<void> DetectCycles() const {
			enum class Mark : uint8_t { Unvisited, OnStack, Done };
			std::unordered_map<const Method*, Mark> marks;

			auto visit = [&marks](auto&& self, const std::shared_ptr<Method>& prototype) -> Result<void> {
				switch (marks[prototype.get()]) {
					case Mark::Done:
						return {};
					case Mark::OnStack:
						return MakeError(
							"Prototype '{}' is part of a reference cycle",
							prototype->_impl->name
						);
					case Mark::Unvisited:
						break;
				}

				marks[prototype.get()] = Mark::OnStack;

				auto descend = [&](const Property::Impl& prop) -> Result<void> {
					if (auto nested = DefinitionOf(prop.prototype)) {
						return self(self, nested);
					}
					return {};
				};

				for (const auto& param : prototype->_impl->paramTypes) {
					if (auto result = descend(*param._impl); !result) {
						return result;
					}
				}

				if (auto result = descend(*prototype->_impl->retType._impl); !result) {
					return result;
				}

				marks[prototype.get()] = Mark::Done;
				return {};
			};

			for (const auto& [_, prototype] : prototypes) {
				if (auto result = visit(visit, prototype); !result) {
					return result;
				}
			}

			return {};
		}
	};
}

Result<void> Manifest::Resolve() {
	TypeTable table;

	// Declared definitions go in first, so that a clash between two of them is
	// reported against the manifest's own tables rather than against whichever
	// inline definition happened to be walked first.
	std::vector<std::shared_ptr<Method>> declared;

	if (prototypes) {
		declared.reserve(prototypes->size());
		for (auto& prototype : *prototypes) {
			if (auto result = table.Register(table.prototypes, &prototype, "prototype", "Manifest"); !result) {
				return MakeError(std::move(result.error()));
			}
			declared.push_back(prototype);
		}
	}

	if (enums) {
		for (auto& enumerate : *enums) {
			if (auto result = table.Register(table.enums, &enumerate, "enum", "Manifest"); !result) {
				return MakeError(std::move(result.error()));
			}
		}
	}

	// Pass one: hoist inline definitions into the table. Collect() descends into
	// each definition it introduces, so walking the methods and the declared
	// prototypes reaches everything.
	if (methods) {
		for (auto& method : *methods) {
			if (auto result = table.Collect(*method._impl, "Method"); !result) {
				return result;
			}
		}
	}

	for (auto& prototype : declared) {
		if (auto result = table.Collect(*prototype->_impl, "Prototype"); !result) {
			return result;
		}
	}

	// Pass two: resolve references, now that every definition is known.
	if (methods) {
		for (auto& method : *methods) {
			if (auto result = table.Link(*method._impl, "Method"); !result) {
				return result;
			}
		}
	}

	for (const auto& [_, prototype] : table.prototypes) {
		if (auto result = table.Link(*prototype->_impl, "Prototype"); !result) {
			return result;
		}
	}

	if (auto result = table.DetectCycles(); !result) {
		return result;
	}

	// Publish the merged tables, sorted by name so that consumers which generate
	// code from a manifest get a stable ordering.
	auto publish = [](auto& field, const auto& table) {
		if (table.empty()) {
			return;
		}

		typename std::remove_reference_t<decltype(field)>::value_type merged;
		merged.reserve(table.size());
		for (const auto& [_, definition] : table) {
			merged.push_back(definition);
		}
		std::ranges::sort(merged, {}, [](const auto& definition) -> const std::string& {
			return definition->_impl->name;
		});
		field = std::move(merged);
	};

	publish(prototypes, table.prototypes);
	publish(enums, table.enums);

	return {};
}

// Checks what the JSON schema for this extension type could not: names that have
// to be unique across an array, fields that constrain one another, and the links
// Resolve() was supposed to have made. Everything about the shape of a manifest -
// required fields, types, patterns - was settled before this runs, so a manifest
// that never went through the schema is not fully checked by this alone.
Result<void> Manifest::Validate() const {
	if (methods) {
		std::unordered_set<std::string_view> methodNames;
		std::unordered_set<std::string_view> functionNames;

		methodNames.reserve(methods->size());
		functionNames.reserve(methods->size());

		for (const auto& method : *methods) {
			// Two methods under one name would make the exported API ambiguous;
			// two names for one funcName would export the same symbol twice.
			if (!methodNames.insert(method._impl->name).second) {
				return MakeError("Duplicate method name: {}", method._impl->name);
			}

			if (!functionNames.insert(method._impl->funcName).second) {
				return MakeError("Duplicate function name: {}", method._impl->funcName);
			}

			if (auto result = ValidateMethod(*method._impl, "Method"); !result) {
				return result;
			}
		}
	}

	// Every prototype and enum in the manifest lives in these tables once
	// Resolve() has run, inline ones included, so validating them here covers
	// the definitions that paramTypes and retType point at.
	if (prototypes) {
		for (const auto& prototype : *prototypes) {
			if (auto result = ValidateMethod(*prototype->_impl, "Prototype"); !result) {
				return result;
			}
		}
	}

	if (enums) {
		for (const auto& enumerate : *enums) {
			if (auto result = ValidateEnumObject(*enumerate->_impl); !result) {
				return result;
			}
		}
	}

	if (classes) {
		std::unordered_set<std::string_view> classNames;
		classNames.reserve(classes->size());

		for (const auto& classObj : *classes) {
			if (!classNames.insert(classObj._impl->name).second) {
				return MakeError("Duplicate class name: {}", classObj._impl->name);
			}

			if (auto result = ValidateClass(*classObj._impl); !result) {
				return result;
			}
		}
	}

	return {};
}

void Manifest::ResolvePaths(const std::filesystem::path& base, const std::filesystem::path& file) {
	auto create_name = [](const std::filesystem::path& path) -> std::filesystem::path {
		return std::format(PLUGIFY_PATH_LITERAL("" PLUGIFY_LIBRARY_PREFIX "{}" PLUGIFY_LIBRARY_SUFFIX), path.stem().native());
	};

	// Language module library must be named 'lib${name}(.dylib|.so|.dll)'.
	if (runtime) {
		runtime->replace_filename(create_name(*runtime));
	} else {
		runtime = PLUGIFY_PATH_LITERAL("bin") / create_name(file);
	}
	runtime = base / *runtime;

	// Set correct path to directories
	if (directories) {
		for (auto& dir : *directories) {
			dir = base / dir;
		}
	}
}
