#pragma once

#include "plugify/binding.hpp"
#include "plugify/alias.hpp"

namespace plugify {
	struct Binding::Impl  {
		std::string name;
		std::optional<bool> bindSelf;
		std::optional<std::map<size_t, Alias>> paramAliases;
		std::optional<Alias> retAlias;
	};
}
