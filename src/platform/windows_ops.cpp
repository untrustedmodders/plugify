#include "plugify/platform_ops.hpp"

#pragma region WinApi
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#ifndef NOGDICAPMASKS
#define NOGDICAPMASKS 1 // CC_*, LC_*, PC_*, CP_*, TC_*, RC_*
#endif

#ifndef NOVIRTUALKEYCODES
#define NOVIRTUALKEYCODES 1 // VK_*
#endif

#ifndef NOWINMESSAGES
#define NOWINMESSAGES 1 // WM_*, EM_*, LB_*, CB_*
#endif

#ifndef NOWINSTYLES
#define NOWINSTYLES 1 // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#endif

#ifndef NOSYSMETRICS
#define NOSYSMETRICS 1 // SM_*
#endif

#ifndef NOMENUS
#define NOMENUS 1 // MF_*
#endif

#ifndef NOICONS
#define NOICONS 1 // IDI_*
#endif

#ifndef NOKEYSTATES
#define NOKEYSTATES 1 // MK_*
#endif

#ifndef NOSYSCOMMANDS
#define NOSYSCOMMANDS 1 // SC_*
#endif

#ifndef NORASTEROPS
#define NORASTEROPS 1 // Binary and Tertiary raster ops
#endif

#ifndef NOSHOWWINDOW
#define NOSHOWWINDOW 1 // SW_*
#endif

#ifndef OEMRESOURCE
#define OEMRESOURCE 1 // OEM Resource values
#endif

#ifndef NOATOM
#define NOATOM 1 // Atom Manager routines
#endif

#ifndef NOCLIPBOARD
#define NOCLIPBOARD 1 // Clipboard routines
#endif

#ifndef NOCOLOR
#define NOCOLOR 1 // Screen colors
#endif

#ifndef NOCTLMGR
#define NOCTLMGR 1 // Control and Dialog routines
#endif

#ifndef NODRAWTEXT
#define NODRAWTEXT 1 // DrawText() and DT_*
#endif

#ifndef NOGDI
#define NOGDI 1 // All GDI defines and routines
#endif

#ifndef NOKERNEL
#define NOKERNEL 1 // All KERNEL defines and routines
#endif

#ifndef NOUSER
#define NOUSER 1 // All USER defines and routines
#endif

#ifndef NONLS
#define NONLS 1 // All NLS defines and routines
#endif

#ifndef NOMB
#define NOMB 1 // MB_* and MessageBox()
#endif

#ifndef NOMEMMGR
#define NOMEMMGR 1 // GMEM_*, LMEM_*, GHND, LHND, associated routines
#endif

#ifndef NOMETAFILE
#define NOMETAFILE 1 // typedef METAFILEPICT
#endif

#ifndef NOMINMAX
#define NOMINMAX 1 // Macros min(a,b) and max(a,b)
#endif

#ifndef NOMSG
#define NOMSG 1 // typedef MSG and associated routines
#endif

#ifndef NOOPENFILE
#define NOOPENFILE 1 // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#endif

#ifndef NOSCROLL
#define NOSCROLL 1 // SB_* and scrolling routines
#endif

#ifndef NOSERVICE
#define NOSERVICE 1 // All Service Controller routines, SERVICE_ equates, etc.
#endif

#ifndef NOSOUND
#define NOSOUND 1 // Sound driver routines
#endif

#ifndef NOTEXTMETRIC
#define NOTEXTMETRIC 1 // typedef TEXTMETRIC and associated routines
#endif

#ifndef NOWH
#define NOWH 1 // SetWindowsHook and WH_*
#endif

#ifndef NOWINOFFSETS
#define NOWINOFFSETS 1 // GWL_*, GCL_*, associated routines
#endif

#ifndef NOCOMM
#define NOCOMM 1 // COMM driver routines
#endif

#ifndef NOKANJI
#define NOKANJI 1 // Kanji support stuff.
#endif

#ifndef NOHELP
#define NOHELP 1 // Help engine interface.
#endif

#ifndef NOPROFILER
#define NOPROFILER 1 // Profiler interface.
#endif

#ifndef NODEFERWINDOWPOS
#define NODEFERWINDOWPOS 1 // DeferWindowPos routines
#endif

#ifndef NOMCX
#define NOMCX 1 // Modem Configuration Extensions
#endif

#ifndef NOWINRES
#define NOWINRES 1
#endif

#ifndef NOIME
#define NOIME 1
#endif
#pragma endregion WinApi

#include <windows.h>

#undef LoadLibrary

namespace plugify {
	class WindowsPlatformOps final : public IPlatformOps {
	private:
		std::unordered_map<std::filesystem::path, DLL_DIRECTORY_COOKIE, plg::path_hash> _searchCookies;
		std::mutex _searchMutex;

		static int TranslateFlags(LoadFlag flags) {
			int winFlags = 0;
			if (flags & LoadFlag::SecureSearch) {
				winFlags |= LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR;
			}
			return winFlags;
		}

		static std::string GetLastErrorString() {
			DWORD error = ::GetLastError();
			if (error == 0) {
				return "Unknown error";
			}

			LPSTR buffer = nullptr;
			size_t size = ::FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<LPSTR>(&buffer),
				0,
				nullptr
			);

			if (size == 0) {
				return std::format("Unknown error ({})", error);
			}

			std::string message(buffer, size);
			::LocalFree(buffer);
			return message;
		}

	public:
		~WindowsPlatformOps() override {
			for (const auto& [path, cookie] : _searchCookies) {
				::RemoveDllDirectory(cookie);
			}
		}

		Result<void*> LoadLibrary(const std::filesystem::path& path, LoadFlag flags) override {
			HMODULE handle = ::LoadLibraryExW(path.c_str(), nullptr, TranslateFlags(flags));
			if (!handle) {
				return MakeError(
					"Failed to load library '{}': {}",
					plg::as_string(path),
					GetLastErrorString()
				);
			}

			if (flags & LoadFlag::NoUnload) {
				HMODULE pinned = nullptr;
				::GetModuleHandleExW(
					GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
					reinterpret_cast<LPCWSTR>(handle),
					&pinned
				);
			}

			return handle;
		}

		Result<void> UnloadLibrary(void* handle) override {
			if (!::FreeLibrary(static_cast<HMODULE>(handle))) {
				return MakeError("Failed to unload library: {}", GetLastErrorString());
			}
			return {};
		}

		Result<Address> GetSymbol(void* handle, std::string_view name) override {
			FARPROC proc = ::GetProcAddress(static_cast<HMODULE>(handle), name.data());
			if (!proc) {
				return MakeError("Symbol '{}' not found: {}", name, GetLastErrorString());
			}
			return proc;
		}

		Result<std::filesystem::path> GetLibraryPath(void* handle) override {
			std::wstring path(MAX_PATH, L'\0');
			DWORD size = ::GetModuleFileNameW(
				static_cast<HMODULE>(handle),
				path.data(),
				static_cast<DWORD>(path.size())
			);

			if (size == 0) {
				return MakeError("Failed to get module path: {}", GetLastErrorString());
			}

			path.resize(size);
			return std::filesystem::path(std::move(path));
		}

		bool SupportsRuntimePathModification() const override {
			return true;
		}

		bool SupportsLazyBinding() const override {
			return false;
		}

		Result<void> AddSearchPath(const std::filesystem::path& path) override {
			DLL_DIRECTORY_COOKIE cookie = ::AddDllDirectory(path.c_str());
			if (!cookie) {
				return MakeError(
					"Failed to add search path '{}': {}",
					plg::as_string(path),
					GetLastErrorString()
				);
			}
			std::lock_guard lock(_searchMutex);
			_searchCookies[path] = cookie;
			return {};
		}

		Result<void> RemoveSearchPath(const std::filesystem::path& path) override {
			auto it = _searchCookies.find(path);
			if (it != _searchCookies.end()) {
				if (!::RemoveDllDirectory(it->second)) {
					return MakeError(
						"Failed to remove search path '{}': {}",
						plg::as_string(path),
						GetLastErrorString()
					);
				}
				std::lock_guard lock(_searchMutex);
				_searchCookies.erase(it);
				return {};
			}
			return MakeError("Failed to find search path '{}'", plg::as_string(path));
		}
	};

	std::shared_ptr<IPlatformOps> CreatePlatformOps() {
		return std::make_shared<WindowsPlatformOps>();
	}
}
