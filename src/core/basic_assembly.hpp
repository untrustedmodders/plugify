#pragma once

#include "plugify/assembly.hpp"
#include "core/assembly_handle.hpp"

namespace plugify {
    class BasicAssembly : public IAssembly {
    private:
        std::unique_ptr<AssemblyHandle> _handle;
        std::shared_ptr<IPlatformOps> _ops;
        //std::unique_ptr<ISymbolResolver> _symbolResolver;
        //std::unique_ptr<IPatternScanner> _patternScanner;

#if PLUGIFY_CACHE_SYMBOLS
        // Symbol cache
        mutable std::shared_mutex _cacheMutex;
        mutable std::unordered_map<std::string, MemAddr, plg::string_hash, std::equal_to<>> _symbolCache;
#endif

    public:
        BasicAssembly() = default;

        explicit BasicAssembly(std::unique_ptr<AssemblyHandle> handle, std::shared_ptr<IPlatformOps> ops)
            : _handle(std::move(handle))
            , _ops(std::move(ops))
            {}

        Result<MemAddr> GetSymbol(std::string_view name) const override {
#if PLUGIFY_CACHE_SYMBOLS
            // Check cache first
            {
                std::shared_lock lock(_cacheMutex);
                auto it = _symbolCache.find(name);
                if (it != _symbolCache.end()) {
                    return it->second;
                }
            }
#endif
            // Resolve symbol
            auto result = _handle->GetSymbol(name);

#if PLUGIFY_CACHE_SYMBOLS
            // Cache if successful
            if (result) {
                std::unique_lock lock(_cacheMutex);
                _symbolCache.emplace(name, *result);
            }
#endif

            return result;
        }

        bool IsValid() const override {
            return _handle && _handle->IsValid();
        }

        const std::filesystem::path& GetPath() const override {
            return _handle ? _handle->GetPath() : emptyPath;
        }

        MemAddr GetBase() const override {
            return _handle ? _handle->GetHandle() : nullptr;
        }

        void* GetHandle() const override {
            return _handle ? _handle->GetHandle() : nullptr;
        }

        static inline const std::filesystem::path emptyPath;
    };
}
