#if PLUGIFY_PLATFORM_SWITCH

#include <plugify/assembly.h>

#include "os.h"

using namespace plugify;

namespace {
	struct Handle {
		nn::ro::Module module{};
		std::unique_ptr<uint8_t> data;
		size_t size{};
		std::unique_ptr<uint8_t> bssData;
		size_t bssSize{};

		Handle(const fs::path& path, LoadFlag flags, std::string& error) {
			nn::fs::FileHandle file;
			nn::Result ret = nn::fs::OpenFile(&file, path.c_str(), nn::fs::OpenMode_Read);
			if (ret.IsFailure()) {
				error = "nn::fs::OpenFile failed. module = %d desc = %d inner_value = 0x%08x", ret.GetModule(), ret.GetDescription(), ret.GetInnerValueForDebug());
				return;
			}

			ret = nn::fs::GetFileSize(&size, file);
			if (ret.IsFailure()) {
				error = "nn::fs::GetFileSize failed. module = %d desc = %d inner_value = 0x%08x", ret.GetModule(), ret.GetDescription(), ret.GetInnerValueForDebug());
				return;
			}

			constexpr auto alignment = 0x1000;

			data = std::unique_ptr<uint8_t>(static_cast<uint8_t*>(std::aligned_alloc(alignment, size)));
			nn::fs::CloseFile(file);

			ret = nn::ro::GetBufferSize(&bssSize, data.get());
			if (ret.IsFailure()) {
				error = "nn::ro::GetBufferSize failed. module = %d desc = %d inner_value = 0x%08x", ret.GetModule(), ret.GetDescription(), ret.GetInnerValueForDebug());
				return;
			}
			bssData = std::unique_ptr<uint8_t>(static_cast<uint8_t*>(std::aligned_alloc(alignment, bssSize)));

			ret = nn::ro::LoadModule(&module, data.get(), bssData.get(), bssSize, static_cast<nn::ro::BindFlag>(TranslateLoading(flags)));
			switch (ret) {
				case nn::ro::ResultInvalidNroImage:
					error = "Data other than an NRO file has been loaded into the specified memory area.";
					size = bssSize = 0;
					break;
				case nn::ro::ResultOutOfAddressSpace:
					error = "There is insufficient free memory area to load the NRO file.";
					size = bssSize = 0;
					break;
				case nn::ro::ResultNroAlreadyLoaded:
					error = "The target NRO file has already been loaded by nn::ro::LoadModule().";
					size = bssSize = 0;
					break;
				case nn::ro::ResultMaxModule:
					error = "The maximum limit has been reached for the number of modules that can be loaded.";
					size = bssSize = 0;
					break;
				case nn::ro::ResultNotAuthorized:
					error = "Authentication failed while loading the file.";
					size = bssSize = 0;
					break;
				default:
					break;
			}
		}

		~Handle() {
			if (size && bssSize) {
				nn::ro::UnloadModule(&module);
			}
		}
	};
}

Assembly::~Assembly() {
	if (_handle) {
		delete static_cast<Handle*>(_handle);
		_handle = nullptr;
	}
}

bool Assembly::InitFromName(std::string_view /*moduleName*/, LoadFlag /*flags*/, bool /*sections*/, bool /*extension*/) {
	// TODO: Implement
	return false;
}

bool Assembly::InitFromMemory(MemAddr /*moduleMemory*/, LoadFlag /*flags*/, bool /*sections*/) {
	// TODO: Implement
	return false;
}

bool Assembly::Init(fs::path modulePath, LoadFlag flags, bool /*sections*/) {
	auto* handle = new Handle(modulePath, flags, _error);
	if (!_error.empty()) {
		delete handle;
		return false;
	}

	_handle = handle;
	_path = std::move(modulePath);

	// TODO: Implement sections

	return true;
}

MemAddr Assembly::GetVirtualTableByName(std::string_view tableName, bool /*decorated*/) const {
	if (tableName.empty())
		return nullptr;

	// TODO: Implement

	return nullptr;
}

MemAddr Assembly::GetFunctionByName(std::string_view functionName) const noexcept {
	if (!_handle)
		return nullptr;

	if (functionName.empty())
		return nullptr;

	uintptr_t address = 0;
	[[maybe_unused]] nn::Result ret = nn::ro::LookupModuleSymbol(&address, &static_cast<const Handle*>(_handle)->module, functionName.data());
#if PLUGIFY_LOGGING
	if (ret.IsFailure()) {
		PL_LOG_VERBOSE("Assembly::GetFunctionByName() - '{}': The symbol could not be found.", functionName);
	}
#endif
	return address;
}

MemAddr Assembly::GetBase() const noexcept {
	return static_cast<const Handle*>(_handle)->module;
}

namespace plugify {
	int TranslateLoading(LoadFlag flags) noexcept {
		int nnFlags = 0;
		if (flags & LoadFlag::Lazy) nnFlags |= nn::ro::BindFlag::BindFlag_Lazy;
		if (flags & LoadFlag::Now) nnFlags |= nn::ro::BindFlag::BindFlag_Now;
		return nnFlags;
	}

	LoadFlag TranslateLoading(int flags) noexcept {
		LoadFlag loadFlags = LoadFlag::Default;
		if (flags & nn::ro::BindFlag::BindFlag_Lazy) loadFlags = loadFlags | LoadFlag::Lazy;
		if (flags & nn::ro::BindFlag::BindFlag_Now) loadFlags = loadFlags | LoadFlag::Now;
		return loadFlags;
	}
}

#endif
