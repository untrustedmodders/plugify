#include "context.hpp"

using namespace plugify;

Context::Context(Plugify& plugify)
	: _plugify{plugify} {
}

Context::~Context() = default;
