#pragma once

#include "plugify/types.hpp"

namespace plugify {
    class Extension;
    // Lifecycle interface
    class IExtensionLifecycle {
    public:
        virtual ~IExtensionLifecycle() = default;
        //virtual void OnReload(Extension& extension) = 0;
        virtual void OnLoad(Extension& extension) = 0;
        virtual void OnUnload(Extension& extension) = 0;
        //virtual void OnEnable(Extension& extension) = 0;
        //virtual void OnDisable(Extension& extension) = 0;
        virtual void OnStart(Extension& extension) = 0;
        virtual void OnEnd(Extension& extension) = 0;
        virtual void OnUpdate(Extension& extension, Duration deltaTime) = 0;
    };
}