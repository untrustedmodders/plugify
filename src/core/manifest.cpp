#include "plugify/file_system.hpp"

#include "core/conflict_impl.hpp"
#include "core/dependency_impl.hpp"
#include "core/enum_object_impl.hpp"
#include "core/enum_value_impl.hpp"
#include "core/glaze_manifest_parser.hpp"
#include "core/method_impl.hpp"

using namespace plugify;

namespace {
	// Helper function to validate a name (alphanumeric, underscore)
	bool IsValidName(const std::string& name) {
		if (name.empty()) {
			return false;
		}
		static const std::regex nameRegex("^[A-Za-z0-9_]+$");
		return std::regex_match(name, nameRegex);
	}

	// Helper function to validate a name (alphanumeric, underscore, dot, dash)
	bool IsValidName2(const std::string& name) {
		if (name.empty()) {
			return false;
		}
		static const std::regex nameRegex("^[A-Za-z0-9_.-]+$");
		return std::regex_match(name, nameRegex);
	}

	// Helper function to validate URL
	bool IsValidURL(const std::string& url) {
		static const std::regex urlRegex(R"(^(https?://)?([a-zA-Z0-9-]+\.)+[a-zA-Z]{2,}(/.*)?$)");
		return std::regex_match(url, urlRegex);
	}

	// Helper function to validate platform
	bool IsValidPlatform(const std::string& platform) {
		/*static const std::unordered_set<std::string> validPlatforms = {
			"windows", "linux", "macos", "android", "ios",
			"unix", "posix", "win32", "win64", "x86", "x64", "arm", "arm64"
		};*/
		if (platform.empty()) {
			return true;
		}
		static const std::regex platformRegex("^[a-z0-9]+(_(x64|arm64|x86|arm32|riscv32|riscv64))$");
		return std::regex_match(platform, platformRegex);
	}

	// Validate EnumObject
	Result<void> ValidateEnumObject(const EnumObject::Impl& enumObj) {
		if (enumObj.name.empty()) {
			return MakeError("Enum name cannot be empty");
		}

		if (!IsValidName(enumObj.name)) {
			return MakeError("Invalid enum name: {}", enumObj.name);
		}

		if (enumObj.values.empty()) {
			return MakeError("Enum '{}' must have at least one value", enumObj.name);
		}

		std::unordered_set<std::string> valueNames;
		//std::unordered_set<int64_t> valueNumbers;

		valueNames.reserve(enumObj.values.size());
		//valueNumbers.reserve(enumObj.values.size());

		for (const auto& v : enumObj.values) {
			const auto& value = *v._impl;

			if (value.name.empty()) {
				return MakeError("Enum value name cannot be empty in enum '{}'", enumObj.name);
			}

			if (!IsValidName(value.name)) {
				return MakeError("Invalid enum value name: {} in enum '{}'", value.name, enumObj.name);
			}

			if (!valueNames.insert(value.name).second) {
				return MakeError("Duplicate enum value name: {} in enum '{}'", value.name, enumObj.name);
			}

			/*if (!valueNumbers.insert(value.value).second) {
				return MakeError(
					"Duplicate enum value: {} for '{}' in enum '{}'",
					value.value,
					value.name,
					enumObj.name
				);
			}*/
		}

		return {};
	}

	extern Result<void> ValidateMethod(const Method::Impl& method, const std::string& context = "");

	// Validate Property
	// The prototype and enum a property points at are validated once via the
	// manifest's shared tables, which Resolve() has already populated with every
	// definition; recursing into them here would both repeat that work and hang
	// on prototypes that refer to one another.
	Result<void> ValidateProperty(const Property::Impl& prop, const std::string& context, bool param) {
		// Check if type is valid (you'd need to define valid ValueType values)
		// Assuming ValueType is an enum, check if it's in valid range

		// If type is void, something wrong
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

		// If type is function/callback, prototype should be present
		if (prop.type == ValueType::Function && !DefinitionOf(prop.prototype)) {
			return MakeError("{}: Function type requires prototype", context);
		}

		// If type is enum, enumerate should be present
		// if (prop.type == ValueType::Enum && !prop.enumerate) {
		//     return context + ": Enum type requires enumerate definition";
		// }

		return {};
	}

	// Validate Method
	Result<void> ValidateMethod(const Method::Impl& method, const std::string& context) {
		std::string prefix = context.empty() ? "Method" : context;

		if (method.name.empty()) {
			return MakeError("{}: name cannot be empty", prefix);
		}

		if (!IsValidName(method.name)) {
			return MakeError("{}: invalid name '{}'", prefix, method.name);
		}

		if (context.empty()) {
			if (method.funcName.empty()) {
				return MakeError("{} '{}': funcName cannot be empty", prefix, method.name);
			}

			if (!IsValidName2(method.funcName)) {
				return MakeError("{} '{}': invalid funcName '{}'", prefix, method.name, method.funcName);
			}
		}

		// Validate varIndex
		constexpr uint8_t kNoVarArgs = 255;	 // Assuming this constant
		if (method.varIndex != kNoVarArgs && method.varIndex >= method.paramTypes.size()) {
			return MakeError("{} '{}': varIndex out of range", prefix, method.name);
		}

		// Validate parameter types
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

		// Validate return type
		if (auto result = ValidateProperty(
				*method.retType._impl,
				std::format("{} '{}' return type", prefix, method.name),
				false
			);
			!result) {
			return result;
		}

		return {};
	}

	// Validate Dependency
	Result<void> ValidateDependency(const Dependency::Impl& dep) {
		if (dep.name.empty()) {
			return MakeError("Dependency name cannot be empty");
		}

		if (!IsValidName(dep.name)) {
			return MakeError("Invalid dependency name: ", dep.name);
		}

		// Validate constraints if present
		// You would need to implement constraint validation based on your Constraint structure
		// if (dep.constraints) {
		//     if (auto result = ValidateConstraint(*dep.constraints); !result) {
		//         return MakeError("Dependency '" + dep.name + "': " + *error);
		//     }
		// }

		return {};
	}

	// Validate Conflict
	Result<void> ValidateConflict(const Conflict::Impl& conflict) {
		if (conflict.name.empty()) {
			return MakeError("Conflict name cannot be empty");
		}

		if (!IsValidName(conflict.name)) {
			return MakeError("Invalid conflict name: ", conflict.name);
		}

		// Validate constraints if present
		// if (conflict.constraints) {
		//     if (auto result = ValidateConstraint(*conflict.constraints); !result) {
		//         return MakeError("Conflict '" + conflict.name + "': " + *error);
		//     }
		// }

		return {};
	}

	// Validate Obsolete
	Result<void> ValidateObsolete(const Obsolete::Impl& obsolete) {
		if (obsolete.name.empty()) {
			return MakeError("Obsolete name cannot be empty");
		}

		if (!IsValidName(obsolete.name)) {
			return MakeError("Invalid obsolete name: ", obsolete.name);
		}

		return {};
	}

	// Validate Alias
	Result<void> ValidateAlias(const Alias::Impl& alias, const std::string& context) {
		if (alias.name.empty()) {
			return MakeError("{}: Alias name cannot be empty", context);
		}

		if (!IsValidName(alias.name)) {
			return MakeError("{}: Invalid alias name '{}'", context, alias.name);
		}

		return {};
	}

	// Validate Binding
	Result<void> ValidateBinding(const Binding::Impl& binding, const std::string& context, const std::optional<ValueType>& handleType = std::nullopt) {
		if (binding.name.empty()) {
			return MakeError("{}: Binding name cannot be empty", context);
		}

		if (!IsValidName(binding.name)) {
			return MakeError("{}: Invalid binding name '{}'", context, binding.name);
		}

		if (binding.method.empty()) {
			return MakeError("{}: Binding '{}' method cannot be empty", context, binding.name);
		}

		if (!IsValidName2(binding.method)) {
			return MakeError("{}: Binding '{}' has invalid method name '{}'", context, binding.name, binding.method);
		}

		// Check if handleless classes have instance methods
		bool isHandleless = handleType.value_or(ValueType::Void) == ValueType::Void;
		if (isHandleless && binding.bindSelf.value_or(false)) {
			return MakeError("{}: Binding '{}': handleless classes (handleType is void/empty) cannot have instance methods (bindSelf=true)", context, binding.name);
		}

		// Validate parameter aliases if present
		if (binding.paramAliases) {
			for (size_t i = 0; i < binding.paramAliases->size(); ++i) {
				if (auto alias = (*binding.paramAliases)[i]) {
					if (auto result = ValidateAlias(
							*alias->_impl,
							std::format("{}: Binding '{}' paramAlias[{}]", context, binding.name, i)
						);
						!result) {
						return result;
					}
				}
			}
		}

		// Validate return alias if present
		if (binding.retAlias) {
			if (auto result = ValidateAlias(
					*binding.retAlias->_impl,
					std::format("{}: Binding '{}' retAlias", context, binding.name)
				);
				!result) {
				return result;
			}
		}

		return {};
	}

	// Validate Class
	Result<void> ValidateClass(const Class::Impl& classObj) {
		if (classObj.name.empty()) {
			return MakeError("Class name cannot be empty");
		}

		if (!IsValidName(classObj.name)) {
			return MakeError("Invalid class name: {}", classObj.name);
		}

		// Check if this is a handleless class
		bool isHandleless = classObj.handleType.value_or(ValueType::Void) == ValueType::Void;

		// Handleless classes cannot have constructors or destructors
		if (isHandleless && (classObj.constructors || classObj.destructor)) {
			return MakeError("Class '{}': handleless classes cannot have constructors or destructors", classObj.name);
		}

		// Validate constructors if present
		if (classObj.constructors) {
			/*if (classObj.constructors->empty()) {
				return MakeError("Class '{}': constructors list cannot be empty if specified", classObj.name);
			}*/

			for (size_t i = 0; i < classObj.constructors->size(); ++i) {
				const auto& constructor = (*classObj.constructors)[i];
				if (constructor.empty()) {
					return MakeError("Class '{}': constructor[{}] cannot be empty", classObj.name, i);
				}

				if (!IsValidName2(constructor)) {
					return MakeError("Class '{}': invalid constructor[{}] name '{}'", classObj.name, i, constructor);
				}
			}
		}

		// Validate destructor if present
		if (classObj.destructor) {
			if (classObj.destructor->empty()) {
				return MakeError("Class '{}': destructor cannot be empty", classObj.name);
			}

			if (!IsValidName2(*classObj.destructor)) {
				return MakeError("Class '{}': invalid destructor name '{}'", classObj.name, *classObj.destructor);
			}
		}

		// Validate bindings
		if (classObj.bindings.empty()) {
			return MakeError("Class '{}': must have at least one binding", classObj.name);
		}

		std::unordered_set<std::string> bindingNames;
		bindingNames.reserve(classObj.bindings.size());

		for (const auto& binding : classObj.bindings) {
			if (!bindingNames.insert(binding._impl->name).second) {
				return MakeError("Class '{}': duplicate binding name '{}'", classObj.name, binding._impl->name);
			}

			if (auto result = ValidateBinding(
					*binding._impl,
					std::format("Class '{}'", classObj.name),
					classObj.handleType
				);
				!result) {
				return result;
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
			const std::string& context
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
		Result<void> Collect(Property::Impl& prop, const std::string& context) {
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

		Result<void> Collect(Method::Impl& method, const std::string& context) {
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
			const std::string& context
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

		Result<void> Link(Method::Impl& method, const std::string& context) {
			auto link = [&](Property::Impl& prop, const std::string& where) -> Result<void> {
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
			if (auto result = table.Register(table.prototypes, &prototype, "prototype", "Manifest");
				!result) {
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

// Main Manifest validation implementation
Result<void> Manifest::Validate() const {
	// Validate required common fields
	if (name.empty()) {
		return MakeError("Manifest name is required");
	}

	if (!IsValidName2(name)) {
		return MakeError("Invalid manifest name: {}", name);
	}

	// Validate version (assuming Version has its own validation)
	// if (!version.IsValid()) {
	//     return MakeError("Invalid version format");
	// }

	if (language.empty()) {
		return MakeError("Language cannot be empty if specified");
	}

	if (!IsValidName(language)) {
		return MakeError("Invalid language name: {}", language);
	}

	// Validate optional common fields
	if (author && author->empty()) {
		return MakeError("Author cannot be empty if specified");
	}

	if (website && !IsValidURL(*website)) {
		return MakeError("Invalid website URL: {}", *website);
	}

	if (license && license->empty()) {
		return MakeError("License cannot be empty if specified");
	}

	// Validate platforms
	if (platforms) {
		for (const auto& platform : *platforms) {
			if (!IsValidPlatform(platform)) {
				return MakeError("Invalid platform: {}", platform);
			}
		}
	}

	// Validate dependencies
	if (dependencies) {
		for (const auto& dep : *dependencies) {
			if (auto result = ValidateDependency(*dep._impl); !result) {
				return result;
			}
		}
	}

	// Validate conflicts
	if (conflicts) {
		for (const auto& conflict : *conflicts) {
			if (auto result = ValidateConflict(*conflict._impl); !result) {
				return result;
			}
		}
	}

	// Validate obsoletes
	if (obsoletes) {
		for (const auto& obsolete : *obsoletes) {
			if (auto result = ValidateObsolete(*obsolete._impl); !result) {
				return result;
			}
		}
	}

	// Determine manifest type based on fields present
	bool hasPluginFields = entry.has_value() || methods.has_value() || classes.has_value()
						   || prototypes.has_value() || enums.has_value();
	bool hasModuleFields = runtime.has_value() || directories.has_value();

	if (hasPluginFields && hasModuleFields) {
		return MakeError("Manifest cannot have both plugin and module fields");
	}

	// Validate plugin-specific fields
	if (hasPluginFields) {
		if (entry && entry->empty()) {
			return MakeError("Plugin entry point cannot be empty");
		}

		/*if (entry && !IsValidName(*entry)) {
			return MakeError("Invalid plugin entry point: {}", *entry);
		}*/

		if (methods) {
			/*if (methods->empty()) {
				return MakeError("Methods list cannot be empty if specified");
			}*/

			std::unordered_set<std::string> methodNames;
			std::unordered_set<std::string> functionNames;

			methodNames.reserve(methods->size());
			functionNames.reserve(methods->size());

			for (const auto& method : *methods) {
				if (!methodNames.insert(method._impl->name).second) {
					return MakeError("Duplicate method name: {}", method._impl->name);
				}

				if (!functionNames.insert(method._impl->funcName).second) {
					return MakeError("Duplicate function name: {}", method._impl->funcName);
				}

				if (auto result = ValidateMethod(*method._impl); !result) {
					return result;
				}
			}
		}

		// Every prototype and enum in the manifest lives in these tables once
		// Resolve() has run, inline ones included, so validating them here covers
		// the definitions that paramTypes and retType point at.
		if (prototypes) {
			for (const auto& prototype : *prototypes) {
				if (auto result = ValidateMethod(
						*prototype->_impl,
						std::format("Prototype '{}'", prototype->_impl->name)
					);
					!result) {
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

		// Add this new block for classes validation:
		if (classes) {
			/*if (classes->empty()) {
				return MakeError("Classes list cannot be empty if specified");
			}*/

			std::unordered_set<std::string> classNames;
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
	}

	// Validate module-specific fields
	if (hasModuleFields) {
		if (runtime) {
			/*if (runtime->empty()) {
				return MakeError("Module runtime path cannot be empty");
			}*/

			// Check if runtime file exists (optional, might want to make this configurable)
			/*if (!fs->IsExists(*runtime)) {
				return MakeError("Runtime file does not exist: {}", plg::as_string(*runtime));
			}

			// Check if it's a regular file
			if (!fs->IsRegularFile(*runtime)) {
				return MakeError("Runtime path is not a file: {}", plg::as_string(*runtime));
			}*/
		}

		if (directories) {
			if (directories->empty()) {
				return MakeError("Directories list cannot be empty if specified");
			}

			for (const auto& dir : *directories) {
				if (dir.empty()) {
					return MakeError("Directory path cannot be empty");
				}

				// Check if directory exists (optional)
				/*if (!fs->IsExists(dir)) {
					return MakeError("Directory does not exist: {}", plg::as_string(dir));
				}

				if (!fs->IsDirectory(dir)) {
					return MakeError("Path is not a directory: {}", plg::as_string(dir));
				}*/
			}
		}
	}

	// All validations passed
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
