#pragma once

#include <vector>
#include <memory>

#include "plugify/asm/assembly.hpp"

namespace plugify {
	/**
	 * @class IAssemblyLoader
	 * @brief Interface for loading assemblies.
	 */
	class AssemblyLoader : public IAssemblyLoader {
	public:
		~AssemblyLoader() override = default;

		Result<std::unique_ptr<IAssembly>> Load(const std::filesystem::path& path, LoadFlag flags) override;

		bool AddSearchPath(const std::filesystem::path& path) override;

		bool CanLinkSearchPaths() const override;
	};
} // namespace plugify