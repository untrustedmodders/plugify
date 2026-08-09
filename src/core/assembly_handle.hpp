#pragma once

#include "plugify/platform_ops.hpp"

namespace plugify {
	// RAII wrapper for platform handles
	class AssemblyHandle {
		void* _handle = nullptr;
		std::shared_ptr<IPlatformOps> _ops;
		std::filesystem::path _path;

	public:
		AssemblyHandle() = default;

		AssemblyHandle(void* handle, std::shared_ptr<IPlatformOps> ops, std::filesystem::path path)
			: _handle(handle)
			, _ops(std::move(ops))
			, _path(std::move(path)) {
		}

		~AssemblyHandle() {
			if (_handle && _ops) {
				[[maybe_unused]] auto _ = _ops->UnloadLibrary(_handle);
			}
		}

		// Delete copy operations
		AssemblyHandle(const AssemblyHandle&) = delete;
		AssemblyHandle& operator=(const AssemblyHandle&) = delete;

		// Move operations
		AssemblyHandle(AssemblyHandle&& other) noexcept
			: _handle(std::exchange(other._handle, nullptr))
			, _ops(std::move(other._ops))
			, _path(std::move(other._path)) {
		}

		AssemblyHandle& operator=(AssemblyHandle&& other) noexcept {
			if (this != &other) {
				_handle = std::exchange(other._handle, nullptr);
				_ops = std::move(other._ops);
				_path = std::move(other._path);
			}
			return *this;
		}

		void* GetHandle() const {
			return _handle;
		}

		const std::filesystem::path& GetPath() const {
			return _path;
		}

		bool IsValid() const {
			return _handle != nullptr;
		}

		Result<Address> GetSymbol(std::string_view name) const {
			return _ops->GetSymbol(_handle, name);
		}
	};
}
