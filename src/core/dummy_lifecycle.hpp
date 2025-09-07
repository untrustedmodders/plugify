#pragma once
#include "plugify/lifecycle.hpp"

namespace plugify {
    class DummyLifecycle : public IExtensionLifecycle {
    public:
        // ILifecycle overrides — intentionally no-op
        void OnLoad([[maybe_unused]] Extension& extension) override {}
        void OnUnload([[maybe_unused]] Extension& extension) override {}
        void OnStart([[maybe_unused]] Extension& extension) override {}
        void OnEnd([[maybe_unused]] Extension& extension) override {}
        void OnUpdate([[maybe_unused]] Extension& extension, [[maybe_unused]] std::chrono::milliseconds deltaTime) override {}
    };
}