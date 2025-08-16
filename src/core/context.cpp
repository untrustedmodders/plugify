#include "context.hpp"
#include <plugify/api/plugify.hpp>

using namespace plugify;

Context::Context(Plugify& plugify)
	: _plugify{plugify} {
}

Context::~Context() = default;
