#pragma once

#include <chrono>

#include "plugify/types.hpp"

namespace plugify {
	class Extension;

	class IExtensionLifecycle {
	public:
		virtual ~IExtensionLifecycle() = default;
		virtual void OnLoad(Extension& extension) = 0;
		virtual void OnUnload(Extension& extension) = 0;
		virtual void OnStart(Extension& extension) = 0;
		virtual void OnEnd(Extension& extension) = 0;
		virtual void OnUpdate(Extension& extension, std::chrono::milliseconds deltaTime) = 0;
		virtual void OnExport(Extension& extension) = 0;
	};
}
