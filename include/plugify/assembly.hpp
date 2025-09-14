#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "plugify/load_flag.hpp"
#include "plugify/mem_addr.hpp"
#include "plugify/types.hpp"

#include "plg/expected.hpp"

namespace plugify {
	/**
	 * @interface IAssembly
	 * @brief Core interface for loaded assemblies
	 */
	class IAssembly {
	public:
		virtual ~IAssembly() = default;

		/**
		 * @brief Get a symbol by name
		 * @param name Symbol name to lookup
		 * @return Address of the symbol or error
		 */
		virtual Result<MemAddr> GetSymbol(std::string_view name) const = 0;

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
		 * @brief Get the base address of the assembly
		 * @return Base memory address
		 */
		virtual MemAddr GetBase() const = 0;

		/**
		 * @brief Get raw platform handle
		 * @return Platform-specific handle (HMODULE, void*, etc)
		 */
		virtual void* GetHandle() const = 0;
	};

	using AssemblyPtr = std::shared_ptr<IAssembly>;
}  // namespace plugify
