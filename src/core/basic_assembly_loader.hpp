#pragma once

#include "plugify/assembly.hpp"
#include "plugify/assembly_loader.hpp"
#include "plugify/file_system.hpp"

#include "core/basic_assembly.hpp"
#include "core/defer.hpp"

namespace plugify {
	class BasicAssemblyLoader : public IAssemblyLoader {
	private:
		std::shared_ptr<IPlatformOps> _ops;
		std::shared_ptr<IFileSystem> _fs;

		// Assembly cache
		mutable std::shared_mutex _cacheMutex;
		std::unordered_map<std::filesystem::path, std::weak_ptr<IAssembly>, plg::path_hash> _cache;

		Result<std::filesystem::path> ResolvePath(const std::filesystem::path& path) const {
			// If absolute path, just verify it exists
			if (path.is_absolute()) {
				if (_fs->IsExists(path)) {
					return path;
				}
				return MakeError("File not found: {}", plg::as_string(path));
			}

			return _fs->GetAbsolutePath(path);
		}

	public:
		BasicAssemblyLoader(std::shared_ptr<IPlatformOps> ops, std::shared_ptr<IFileSystem> fs)
		    : _ops(std::move(ops))
		    , _fs(std::move(fs)) {
		}

		Result<AssemblyPtr> Load(
		    const std::filesystem::path& path,
		    LoadFlag flags,
		    std::span<const std::filesystem::path> searchPaths
		) override {
			// Resolve path
			auto resolvedPath = ResolvePath(path);
			if (!resolvedPath) {
				return MakeError(std::move(resolvedPath.error()));
			}

			// Check cache
			{
				std::shared_lock lock(_cacheMutex);
				auto it = _cache.find(*resolvedPath);
				if (it != _cache.end()) {
					if (auto cached = it->second.lock()) {
						return cached;
					}
				}
			}

			bool supportRuntimePaths = _ops->SupportsRuntimePathModification()
			                           && !searchPaths.empty();

			defer {
				if (supportRuntimePaths) {
					for (const auto& searchPath : searchPaths) {
						[[maybe_unused]] auto removeResult = _ops->RemoveSearchPath(searchPath);
					}
				}
			};

			if (supportRuntimePaths) {
				for (const auto& searchPath : searchPaths) {
					auto addResult = _ops->AddSearchPath(searchPath);
					if (!addResult) {
						return MakeError(std::move(addResult.error()));
					}
				}
			}

			// Load library
			auto handleResult = _ops->LoadLibrary(*resolvedPath, flags);
			if (!handleResult) {
				return MakeError(std::move(handleResult.error()));
			}

			// Create assembly
			auto handle = std::make_unique<AssemblyHandle>(*handleResult, _ops, *resolvedPath);

			auto assembly = std::make_shared<BasicAssembly>(std::move(handle), _ops);

			// Update cache
			{
				std::unique_lock lock(_cacheMutex);
				_cache.emplace(std::move(*resolvedPath), assembly);
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
}