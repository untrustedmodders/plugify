#pragma once

#include "plugify/assembly.hpp"

#include "core/assembly_handle.hpp"

namespace plugify {
	class BasicAssembly final : public IAssembly {
	private:
		std::unique_ptr<AssemblyHandle> _handle;
		std::shared_ptr<IPlatformOps> _ops;

		explicit BasicAssembly(std::unique_ptr<AssemblyHandle> handle, std::shared_ptr<IPlatformOps> ops)
			: _handle(std::move(handle))
			, _ops(std::move(ops)) {
		}

		Result<Address> GetSymbol(std::string_view name) const override {
			return _handle ? _handle->GetSymbol(name) : nullptr;
		}

		bool IsValid() const override {
			return _handle && _handle->IsValid();
		}

		const std::filesystem::path& GetPath() const override {
			return _handle ? _handle->GetPath() : emptyPath;
		}

		Address GetBase() const override {
			return _handle ? _handle->GetHandle() : nullptr;
		}

		void* GetHandle() const override {
			return _handle ? _handle->GetHandle() : nullptr;
		}

		inline static const std::filesystem::path emptyPath;
	};
}
