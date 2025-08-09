#pragma once

#include <vector>
#include <memory>

#include <plugify/asm/assembly.hpp>

namespace plugify {
	/**
	 * @class IAssemblyLoader
	 * @brief Interface for loading assemblies.
	 */
	class AssemblyLoader final : public IAssemblyLoader {
	public:
		AssemblyResult Load(std::filesystem::path_view path, LoadFlag flags) override;

		bool AddSearchPath(std::filesystem::path_view path) override;

	private:
		std::vector<std::filesystem::path> _searchPaths;
	};
} // namespace plugify