#pragma once

namespace plugify {
	class IPlugify;

	class PlugifyContext {
	public:
		explicit PlugifyContext(std::weak_ptr<IPlugify> plugify);
		~PlugifyContext();

	protected:
		std::weak_ptr<IPlugify> _plugify;
	};
}
