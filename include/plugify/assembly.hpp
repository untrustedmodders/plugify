#pragma once

#include <memory>
#include <string>
#include <filesystem>

#include "plugify/load_flag.hpp"
#include "plugify/address.hpp"
#include "plugify/types.hpp"

#include "plg/expected.hpp"

namespace plugify {
	/**
	 * @interface IAssembly
	 * @brief Core interface for loaded assemblies
	 */
	class IAssembly {
	public:
		/**
		 * @brief Virtual destructor
		 */
		virtual ~IAssembly() = default;

		/**
		 * @brief Get a symbol by name
		 * @param name Symbol name to lookup
		 * @return Address of the symbol or error
		 */
		virtual Result<Address> GetSymbol(std::string_view name) const = 0;

		/**
		 * @brief Check if assembly is valid
		 * @return true if the assembly is loaded and valid
		 */
		virtual bool IsValid() const = 0;

		/**
		 * @brief Get the path of the loaded assembly
		 * @return Filesystem path to the assembly
		 */
		virtual const std::filesystem::path& GetPath() const = 0;

		/**
		 * @brief Get raw platform handle
		 * @return Platform-specific handle (HMODULE, void*, etc)
		 */
		virtual void* GetHandle() const = 0;
	};

	/**
	 * @typedef AssemblyPtr
	 * @brief Shared ownership of a loaded assembly.
	 */
	using AssemblyPtr = std::shared_ptr<IAssembly>;
}  // namespace plugify
