#include <plugify/plugify_provider.h>
#include <core/plugify_provider.h>

using namespace plugify;

IPlugifyProvider::IPlugifyProvider(PlugifyProvider& impl) : _impl{impl} {
}

IPlugifyProvider::~IPlugifyProvider() = default;

void IPlugifyProvider::Log(const std::string& msg, Severity severity) const {
    _impl.Log(msg, severity);
}

const fs::path& IPlugifyProvider::GetBaseDir() const {
    return _impl.GetBaseDir();
}
