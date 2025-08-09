#if PLUGIFY_PLATFORM_SWITCH

#include <plugify/assembly.hpp>

#include "os.h"

using namespace plugify;

namespace {
	struct Handle {
		nn::ro::Module module{};
		std::unique_ptr<uint8_t> data;
		int64_t size{};
		std::unique_ptr<uint8_t> bssData;
		size_t bssSize{};

		Handle(const std::filesystem::path& path, LoadFlag flags, std::string& error) {
			nn::fs::FileHandle file;
			nn::Result ret = nn::fs::OpenFile(&file, path.c_str(), nn::fs::OpenMode_Read);
			if (ret.IsFailure()) {
				error = std::format("nn::fs::OpenFile failed. module = {} desc = {} inner_value = 0x{:08x}", ret.GetModule(), ret.GetDescription(), ret.GetInnerValueForDebug());
				return;
			}

			ret = nn::fs::GetFileSize(&size, file);
			if (ret.IsFailure()) {
				error = std::format("nn::fs::GetFileSize failed. module = {} desc = {} inner_value = 0x{:08x}", ret.GetModule(), ret.GetDescription(), ret.GetInnerValueForDebug());
				return;
			}

			data = std::unique_ptr<uint8_t>(static_cast<uint8_t*>(std::aligned_alloc(nn::os::MemoryPageSize, size)));
			nn::fs::CloseFile(file);

			ret = nn::ro::GetBufferSize(&bssSize, data.get());
			if (ret.IsFailure()) {
				error = std::format("nn::ro::GetBufferSize failed. module = {} desc = {} inner_value = 0x{:08x}", ret.GetModule(), ret.GetDescription(), ret.GetInnerValueForDebug());
				return;
			}
			bssData = std::unique_ptr<uint8_t>(static_cast<uint8_t*>(std::aligned_alloc(nn::os::MemoryPageSize, bssSize)));

			ret = nn::ro::LoadModule(&module, data.get(), bssData.get(), bssSize, TranslateLoading(flags));
			if (ret.IsFailure()) {
				if (nn::ro::ResultInvalidNroImage::Includes(ret)) {
					error = "Data other than an NRO file has been loaded into the specified memory area.";
				} else if(nn::ro::ResultOutOfAddressSpace::Includes(ret)) {
					error = "There is insufficient free memory area to load the NRO file.";
				} else if(nn::ro::ResultNroAlreadyLoaded::Includes(ret)) {
					error = "The target NRO file has already been loaded by nn::ro::LoadModule().";
				} else if(nn::ro::ResultMaxModule::Includes(ret)) {
					error = "The maximum limit has been reached for the number of modules that can be loaded.";
				} else if(nn::ro::ResultNotAuthorized::Includes(ret)) {
					error = "Authentication failed while loading the file.";
				} else {
					error = "Unknown error.";
				}
				size = bssSize = 0;
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

bool Assembly::InitFromName(std::string_view /*name*/, LoadFlag /*flags*/, const SearchDirs& /*searchDirs*/, bool /*sections*/, bool /*extension*/) {
	// TODO: Implement
	return false;
}

bool Assembly::InitFromMemory(MemAddr /*memory*/, LoadFlag /*flags*/, const SearchDirs& /*searchDirs*/, bool /*sections*/) {
	// TODO: Implement
	return false;
}

bool Assembly::InitFromHandle(Handle /*handle*/, LoadFlag /*flags*/, const SearchDirs& /*searchDirs*/, bool /*sections*/) {
	// TODO: Implement
	return false;
}

bool Assembly::Init(const fs::path& path, LoadFlag flags, const SearchDirs& /*searchDirs*/, bool sections) {
	if (!IsExist(path)) {
		return false;
	}

	auto* handle = new Handle(path, flags, _error);
	if (!_error.empty()) {
		delete handle;
		return false;
	}

	_handle = handle;
	_path = std::move(path);

	if (!sections)
		return true;

	_sections.emplace_back(".bss", reinterpret_cast<uintptr_t>(handle->bssData.get()), handle->bssSize);

	if (sections) {
		return LoadSections();
	}

	return true;
}

bool Assembly::LoadSections() {
	// TODO: Implement

	return false;
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
#endif // PLUGIFY_LOGGING
	return address;
}

MemAddr Assembly::GetBase() const noexcept {
	return static_cast<const Handle*>(_handle)->data.get();
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

#endif // PLUGIFY_PLATFORM_SWITCH
