#pragma once

#include "plugify/binding.hpp"
#include "plugify/alias.hpp"

namespace plugify {
	struct Binding::Impl  {
		std::string name;
		std::string method;
		std::optional<bool> bindSelf;
		std::optional<std::inplace_vector<std::optional<Alias>, Signature::kMaxFuncArgs>> paramAliases;
		std::optional<Alias> retAlias;
	};
}
