#pragma once

#include "plugify/core/assembly.hpp"
#include "plugify/core/assembly_loader.hpp"
#include "plugify/core/file_system.hpp"

#include "core/assembly_handle.hpp"
#include "core/basic_assembly.hpp"

namespace plugify {
    class BasicAssemblyLoader : public IAssemblyLoader {
    private:
        std::shared_ptr<IPlatformOps> _ops;
        std::shared_ptr<IFileSystem> _fileSystem;
        std::vector<std::filesystem::path> _searchPaths;

        // Assembly cache
        mutable std::shared_mutex _cacheMutex;
        std::unordered_map<std::filesystem::path, std::weak_ptr<IAssembly>> _cache;

        Result<std::filesystem::path> ResolvePath(
            const std::filesystem::path& path
        ) const {
            // If absolute path, just verify it exists
            if (path.is_absolute()) {
                if (_fileSystem->IsExists(path)) {
                    return path;
                }
                return MakeError("File not found: {}", path.string());
            }

            return _fileSystem->GetAbsolutePath(path);
        }

    public:
        BasicAssemblyLoader(std::shared_ptr<IPlatformOps> ops)
            : _ops(std::move(ops)) {
            // Initialize default search paths
            _searchPaths.push_back(std::filesystem::current_path());
        }
        Result<AssemblyPtr> Load(
            const std::filesystem::path& path,
            LoadFlag flags
        ) override {
            // Resolve path
            auto pathResult = ResolvePath(path);
            if (!pathResult) {
                return plg::unexpected(std::move(pathResult.error()));
            }

            auto resolvedPath = *pathResult;

            // Check cache
            {
                std::shared_lock lock(_cacheMutex);
                auto it = _cache.find(resolvedPath);
                if (it != _cache.end()) {
                    if (auto cached = it->second.lock()) {
                        return cached;
                    }
                }
            }

            // Load library
            auto handleResult = _ops->LoadLibrary(resolvedPath, flags);
            if (!handleResult) {
                return plg::unexpected(std::move(handleResult.error()));
            }

            // Create assembly
            auto handle = std::make_unique<AssemblyHandle>(
                *handleResult, _ops, resolvedPath
            );

            auto assembly = std::shared_ptr<IAssembly>(
                CreateAssembly(std::move(handle), _ops)
            );

            // Update cache
            {
                std::unique_lock lock(_cacheMutex);
                _cache[std::move(resolvedPath)] = assembly;
            }

            return assembly;
        }

        Result<void> Unload(const AssemblyPtr& assembly) override {
            if (!assembly) {
                return plg::unexpected("Cannot unload null assembly");
            }

            // Remove from cache
            {
                std::unique_lock lock(_cacheMutex);
                _cache.erase(assembly->GetPath());
            }

            return {};
        }
    };
}