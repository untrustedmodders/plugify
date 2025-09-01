#pragma once

#include "plugify/core/assembly.hpp"
#include "plugify/core/assembly_loader.hpp"
#include "core/basic_assembly.hpp"
#include "core/assembly_handle.hpp"

namespace plugify {
    class BasicAssemblyLoader : public IAssemblyLoader {
    private:
        std::shared_ptr<IPlatformOps> _ops;
        std::vector<std::filesystem::path> _searchPaths;

        // Assembly cache
        mutable std::shared_mutex _cacheMutex;
        std::unordered_map<std::filesystem::path, std::weak_ptr<IAssembly>> _cache;

        Result<std::filesystem::path> ResolvePath(
            const std::filesystem::path& path) const {
            // Implementation from previous artifact
            // ... (same as before)
            return path; // Simplified for brevity
        }

    public:
        BasicAssemblyLoader()
            : _ops(CreatePlatformOps()) {
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
                return MakeError("Failed to resolve path: {}", pathResult.error());
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
                return MakeError("Failed to load: {}", handleResult.error());
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
                _cache[resolvedPath] = assembly;
            }

            return assembly;
        }

        Result<void> Unload(const AssemblyPtr& assembly) override {
            if (!assembly) {
                return MakeError("Cannot unload null assembly");
            }

            // Remove from cache
            {
                std::unique_lock lock(_cacheMutex);
                _cache.erase(assembly->GetPath());
            }

            return {};
        }
    };

    // Factory function
    std::unique_ptr<IAssemblyLoader> CreateAssemblyLoader() {
        return std::make_unique<BasicAssemblyLoader>();
    }
}