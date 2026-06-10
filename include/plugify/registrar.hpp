#pragma once

#include "plugify/global.h"
#include "plugify/types.hpp"

namespace plugify {
	PLUGIFY_API std::string ToString(UniqueId id) noexcept;

	// RAII registrar helper: registers in ctor, unregisters in dtor
	class Registrar {
	public:
		Registrar(UniqueId id, std::string name);
		~Registrar();

		Registrar(const Registrar&) = delete;
		Registrar& operator=(const Registrar&) = delete;
		Registrar(Registrar&&) noexcept;
		Registrar& operator=(Registrar&&) noexcept;

	private:
		UniqueId _id;
	};
}