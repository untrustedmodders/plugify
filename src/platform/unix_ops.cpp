#include <dlfcn.h>

#include "plugify/platform_ops.hpp"

namespace plugify {
	class UnixPlatformOps : public IPlatformOps {
	private:
		static int TranslateFlags(LoadFlag flags) {
			int dlFlags = 0;

			if (flags & LoadFlag::LazyBinding) {
				dlFlags |= RTLD_LAZY;
			} else {
				dlFlags |= RTLD_NOW;
			}

			if (flags & LoadFlag::GlobalSymbols) {
				dlFlags |= RTLD_GLOBAL;
			} else {
				dlFlags |= RTLD_LOCAL;
			}

#ifdef RTLD_NODELETE
			if (flags & LoadFlag::NoUnload) {
				dlFlags |= RTLD_NODELETE;
			}
#endif

#ifdef RTLD_DEEPBIND
			if (flags & LoadFlag::DeepBind) {
				dlFlags |= RTLD_DEEPBIND;
			}
#endif

			return dlFlags;
		}

	public:
		Result<void*> LoadLibrary(const std::filesystem::path& path, LoadFlag flags) override {
			void* handle = ::dlopen(path.c_str(), TranslateFlags(flags));
			if (!handle) {
				return MakeError("Failed to load library '{}': {}", plg::as_string(path), ::dlerror());
			}
			return handle;
		}

		Result<void> UnloadLibrary(void* handle) override {
			if (::dlclose(handle) != 0) {
				return MakeError("Failed to unload library: {}", ::dlerror());
			}
			return {};
		}

		Result<MemAddr> GetSymbol(void* handle, std::string_view name) override {
			// Clear any existing error
			::dlerror();

			void* symbol = ::dlsym(handle, name.data());
			const char* error = ::dlerror();
			if (error) {
				return MakeError("Symbol '{}' not found: {}", name, error);
			}
			return symbol;
		}

		Result<std::filesystem::path> GetLibraryPath(void* handle) override {
			Dl_info info;
			if (::dladdr(handle, &info) && info.dli_fname) {
				return std::filesystem::path(info.dli_fname);
			}
			return MakeError("Failed to get library path");
		}

		bool SupportsRuntimePathModification() const override {
			return false;
		}

		bool SupportsLazyBinding() const override {
			return true;
		}
	};

	std::shared_ptr<IPlatformOps> CreatePlatformOps() {
		return std::make_shared<UnixPlatformOps>();
	}
}
