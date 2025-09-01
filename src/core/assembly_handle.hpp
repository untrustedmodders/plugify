#pragma once

#include "plugify/platform_ops.hpp"

namespace plugify {
    // RAII wrapper for platform handles
    class AssemblyHandle {
        void* _handle = nullptr;
        std::shared_ptr<IPlatformOps> _ops;
        std::filesystem::path _path;
        std::atomic<size_t> _refCount{1};
        
    public:
        AssemblyHandle() = default;
        
        AssemblyHandle(void* handle, std::shared_ptr<IPlatformOps> ops, std::filesystem::path path)
            : _handle(handle), _ops(std::move(ops)), _path(std::move(path)) {}
        
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
            , _path(std::move(other._path))
            , _refCount(other._refCount.load()) {}
        
        AssemblyHandle& operator=(AssemblyHandle&& other) noexcept {
            if (this != &other) {
                if (_handle && _ops) {
                    [[maybe_unused]] auto _ = _ops->UnloadLibrary(_handle);
                }
                _handle = std::exchange(other._handle, nullptr);
                _ops = std::move(other._ops);
                _path = std::move(other._path);
                _refCount = other._refCount.load();
            }
            return *this;
        }

        void* GetHandle() const { return _handle; }
        const std::filesystem::path& GetPath() const { return _path; }
        bool IsValid() const { return _handle != nullptr; }
        
        void AddRef() { _refCount.fetch_add(1, std::memory_order_relaxed); }
        size_t ReleaseRef() { return _refCount.fetch_sub(1, std::memory_order_relaxed); }
        size_t GetRefCount() const { return _refCount.load(); }

        Result<MemAddr> GetSymbol(std::string_view name) const {
            return _ops->GetSymbol(_handle, name);
        }
    };
}
