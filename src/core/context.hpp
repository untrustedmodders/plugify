#pragma once

namespace plugify {
	class Plugify;

	class Context {
	public:
		explicit Context(Plugify& plugify);
		~Context();

	protected:
		Plugify& _plugify;
	};
}
