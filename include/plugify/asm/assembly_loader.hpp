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
		AssemblyResult Load(const std::filesystem::path& path, LoadFlag flags) override;

		bool AddSearchPath(const std::filesystem::path& path) override;

		bool CanLinkSearchPaths() const override;

	private:
		std::vector<std::filesystem::path> _searchPaths;
	};
} // namespace plugify