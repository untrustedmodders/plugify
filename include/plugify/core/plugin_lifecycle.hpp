#pragma once

#include "plugify/core/types.hpp"

namespace plugify {
    class Extension;
    // Lifecycle interface
    class ILifecycle {
    public:
        virtual ~ILifecycle() = default;
        virtual Result<void> OnBeforeLoad(Extension& extension) = 0;
        virtual Result<void> OnAfterLoad(Extension& extension) = 0;
        virtual Result<void> OnBeforeUnload(Extension& extension) = 0;
        virtual Result<void> OnAfterUnload(Extension& extension) = 0;
        /*
        virtual Result<void> OnReload(Extension& extension) = 0;
        virtual Result<void> OnLoad(Extension& extension) = 0;
        virtual Result<void> OnUnload(Extension& extension) = 0;
        virtual Result<void> OnEnable(Extension& extension) = 0;
        virtual Result<void> OnDisable(Extension& extension) = 0;
        virtual Result<void> OnUpdate(Extension& extension, double deltaTime) = 0;*/
    };
}