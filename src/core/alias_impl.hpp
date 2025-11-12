#pragma once

#include "plugify/alias.hpp"

namespace plugify {
	struct Alias::Impl  {
		std::string name;
		std::optional<bool> owner;
	};
}
