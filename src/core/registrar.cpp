#include "plugify/registrar.hpp"

using namespace plugify;

// A thread-safe global registry
namespace {
	std::unordered_map<UniqueId, std::string> registry;
	std::shared_mutex mutex;
}

// Register / unregister
Registrar::Registrar(UniqueId id, std::string name)
	: _id(id) {
	std::unique_lock lock(mutex);
	registry[id] = std::move(name);
}

Registrar::~Registrar() {
	std::unique_lock lock(mutex);
	registry.erase(_id);
}

Registrar::Registrar(Registrar&& o) noexcept
	: _id(o._id) {
	o._id = UniqueId{};
}

Registrar& Registrar::operator=(Registrar&& o) noexcept {
	if (this != &o) {
		_id = o._id;
		o._id = UniqueId{};
	}
	return *this;
}

static std::string unknownString = "<unknown>";

const std::string& plugify::ToString(UniqueId id) noexcept {
	std::shared_lock lock(mutex);
	auto it = registry.find(id);
	if (it != registry.end()) {
		return it->second;
	}
	return unknownString;
}
