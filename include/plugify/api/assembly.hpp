#pragma once

#include <memory>
#include <string>

#include <plg/expected.hpp>
#include <plg/string.hpp>
#include <plugify/api/path.hpp>
#include <plugify/asm/load_flag.hpp>
#include <plugify/asm/mem_addr.hpp>

namespace plugify {
	/**
	 * @class IAssembly
	 * @brief Interface for an assembly (module) within a process.
	 *
	 * This interface provides methods to retrieve the assembly ID and symbols.
	 */
	class IAssembly {
	public:
		virtual ~IAssembly() = default;

		/**
		 * @brief Retrieves a raw symbol pointer by name.
		 * @param name The name of the symbol to retrieve.
		 * @return The memory address of the symbol, or nullptr if not found.
		 */
		virtual MemAddr GetSymbol(std::string_view name) const = 0;
	};

	/**
	 * @typedef AssemblyResult
	 * @brief Result type for assembly loading operations.
	 *
	 * Represents either a successfully loaded assembly or an error.
	 */
	using AssemblyResult = plg::expected<std::unique_ptr<IAssembly>, plg::string>;

	/**
	 * @class IAssemblyLoader
	 * @brief Interface for loading assemblies.
	 *
	 * This interface provides methods to load assemblies and manage search paths.
	 */
	class IAssemblyLoader {
	public:
		virtual ~IAssemblyLoader() = default;

		/**
		 * @brief Loads an assembly from the specified path.
		 * @param path The path to the assembly file.
		 * @param flags Flags for loading the assembly.
		 * @return A unique pointer to the loaded assembly.
		 */
		virtual AssemblyResult Load(std::filesystem::path_view path, LoadFlag flags) = 0;

		/**
		 * @brief Adds a search path for loading assemblies.
		 * @param path The path to add to the search paths.
		 */
		virtual bool AddSearchPath(std::filesystem::path_view path) = 0;

		/**
		 * @brief Checks if the implementation can link search paths for opening new shared libraries.
		 * @return True if linking search paths is supported, false otherwise.
		 */
		 virtual bool CanLinkSearchPaths() const = 0;
	};
} // namespace plugify