#pragma once

#include "plugify/core/types.hpp"

namespace plugify {
    class Plugin;
    // Plugin lifecycle interface
    class IPluginLifecycle {
    public:
        virtual ~IPluginLifecycle() = default;
        virtual Result<void> OnBeforeLoad(Plugin& plugin) = 0;
        virtual Result<void> OnAfterLoad(Plugin& plugin) = 0;
        virtual Result<void> OnBeforeUnload(Plugin& plugin) = 0;
        virtual Result<void> OnAfterUnload(Plugin& plugin) = 0;
        virtual Result<void> OnReload(Plugin& plugin) = 0;
    };
}