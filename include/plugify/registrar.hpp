#pragma once

#include "plugify/global.h"
#include "plugify/types.hpp"

namespace plugify {
	PLUGIFY_API std::string ToDebugString(UniqueId id) noexcept;
	PLUGIFY_API std::string ToShortString(UniqueId id) noexcept;

	// RAII registrar helper: registers in ctor, unregisters in dtor
	class Registrar {
	public:
		// DebugInfo contains the human readable metadata
		struct DebugInfo {
			std::string name;
			ExtensionType type;
			std::string version;
			std::filesystem::path location;
		};

		Registrar(UniqueId id, DebugInfo info);
		~Registrar();

		Registrar(const Registrar&) = delete;
		Registrar& operator=(const Registrar&) = delete;
		Registrar(Registrar&&) noexcept;
		Registrar& operator=(Registrar&&) noexcept;

	private:
		UniqueId _id;
	};
}