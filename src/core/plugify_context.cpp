#include "plugify_context.hpp"
#include <plugify/api/plugify.hpp>

using namespace plugify;

PlugifyContext::PlugifyContext(std::weak_ptr<IPlugify> plugify)
	: _plugify{std::move(plugify)} {
}

PlugifyContext::~PlugifyContext() = default;
