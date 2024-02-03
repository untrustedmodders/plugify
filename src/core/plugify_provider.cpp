#include "plugify_provider.h"

using namespace plugify;

PlugifyProvider::PlugifyProvider(std::weak_ptr<IPlugify> plugify) : IPlugifyProvider(*this), PlugifyContext(std::move(plugify)) {
}

PlugifyProvider::~PlugifyProvider() = default;

void PlugifyProvider::Log(const std::string& msg, Severity severity) {
	if (auto locker = _plugify.lock()) {
		locker->Log(msg, severity);
	}
}

std::weak_ptr<IPluginManager> PlugifyProvider::GetPluginManager() {
	if (auto locker = _plugify.lock()) {
		return locker->GetPluginManager();
	}
	return {};
}
