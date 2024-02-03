#include "plugify_context.h"
#include <plugify/plugify.h>

using namespace plugify;

PlugifyContext::PlugifyContext(std::weak_ptr<IPlugify> plugify) : _plugify{std::move(plugify)} {
}

PlugifyContext::~PlugifyContext() = default;
