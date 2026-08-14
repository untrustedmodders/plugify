#pragma once

#include <variant>

#include "plugify/enum.hpp"
#include "plugify/method.hpp"
#include "plugify/property.hpp"

namespace plugify {
	// A `prototype`/`enum` field as written in a manifest: either a full inline
	// definition or the name of one declared elsewhere. Manifest::Resolve()
	// replaces every name with the definition it denotes, so past that point only
	// the first alternative is ever held. An absent field reads as a null
	// definition, which is what this default-constructs to.
	template <class T>
	using Definition = std::variant<std::shared_ptr<T>, std::string>;

	// Null while the field is absent or still an unresolved name.
	template <class T>
	std::shared_ptr<T> DefinitionOf(const Definition<T>& def) noexcept {
		const auto* definition = std::get_if<std::shared_ptr<T>>(&def);
		return definition ? *definition : nullptr;
	}

	struct Property::Impl {
		ValueType type{};
		std::optional<bool> ref;
		Definition<Method> prototype;
		Definition<Enum> enumerate;
	};
}
