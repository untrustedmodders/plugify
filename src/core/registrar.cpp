#include "plugify/core/registrar.hpp"

using namespace plugify;

using DebugInfo = Registrar::DebugInfo;

// A thread-safe global registry
namespace {
    std::unordered_map<UniqueId, DebugInfo> registry;
    std::shared_mutex mutex;
}

// Register / unregister (used by Registrar)
Registrar::Registrar(UniqueId id, DebugInfo info) : _id(id) {
    std::unique_lock lock(mutex);
    registry.emplace(id, std::move(info));
}

Registrar::~Registrar() {
    std::unique_lock lock(mutex);
    registry.erase(_id);
}

Registrar::Registrar(Registrar&& o) noexcept : _id(o._id) {
    o._id = UniqueId{};
}

Registrar& Registrar::operator=(Registrar&& o) noexcept {
    if (this != &o) {
        // unregister previous if any
        {
            std::unique_lock lock(mutex);
            registry.erase(_id);
        }
        _id = o._id;
        o._id = UniqueId{};
    }
    return *this;
}

// Lookup helper
static DebugInfo* LookupDebugInfo(UniqueId id) {
    std::shared_lock lock(mutex);
    auto it = registry.find(id);
    if (it == registry.end())
        return nullptr;
    return &it->second;
}

std::string plugify::ToDebugString(UniqueId id) noexcept {
    if (auto info = LookupDebugInfo(id)) {
        std::string s = info->name.empty() ? std::format("id#{}", id) : info->name;
        if (info->type != ExtensionType::Unknown) s += std::format(" ({})", plg::enum_to_string(info->type));
        if (!info->version.empty()) s += std::format(" v{}", info->version);
        return s;
    }
    return std::format("id#{}", id);
}

std::string_view plugify::ToShortString(UniqueId id) noexcept {
    if (auto info = LookupDebugInfo(id)) {
        if (!info->name.empty()) return info->name;
    }
    return "";
}