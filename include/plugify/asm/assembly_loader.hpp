#pragma once

#include <vector>
#include <memory>

#include "plugify/asm/assembly.hpp"

namespace plugify {
	/**
	 * @class IAssemblyLoader
	 * @brief Interface for loading assemblies.
	 */
	class AssemblyLoader final : public IAssemblyLoader {
	public:
		Result<std::unique_ptr<IAssembly>> Load(const std::filesystem::path& path, LoadFlag flags) override;

		bool AddSearchPath(const std::filesystem::path& path) override;

		bool CanLinkSearchPaths() const override;

	private:
		std::vector<std::filesystem::path> _searchPaths;
	};
} // namespace plugify