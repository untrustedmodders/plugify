#pragma once

#include <vector>

#include "plugify/assembly.hpp"

namespace plugify {
    /**
     * @interface IAssemblyLoader
     * @brief Interface for assembly loading operations
     */
    class IAssemblyLoader {
    public:
        virtual ~IAssemblyLoader() = default;

        /**
         * @brief Load an assembly from file
         * @param path Path to the assembly file
         * @param flags Loading flags
         * @return Loaded assembly or error
         */
        virtual Result<AssemblyPtr> Load(
            const std::filesystem::path& path,
            LoadFlag flags = LoadFlag::Default,
            std::span<const std::filesystem::path> searchPaths = {}) = 0;

        /**
         * @brief Unload an assembly
         * @param assembly Assembly to unload
         * @return Success or error
         */
        virtual Result<void> Unload(const AssemblyPtr& assembly) = 0;
    };

    using AssemblyLoaderPtr = std::shared_ptr<IAssemblyLoader>;
}