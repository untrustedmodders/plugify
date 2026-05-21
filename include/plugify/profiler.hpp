#pragma once

#include <memory>
#include <string_view>

namespace plugify {
	struct ZoneHandle {
		uint64_t opaque = 0;

		explicit operator bool() const { return opaque != 0; }
	};

	struct ZoneDesc {
		std::string_view name;
		std::string_view function;
		std::string_view file;
		size_t line;
		uint32_t color;

		ZoneDesc(std::string_view name,
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 and __GNUC_MINOR__ >= 8)) || (defined(_MSC_VER) && _MSC_VER > 1925)
			 const std::string_view function = __builtin_FUNCTION(),
			 const std::string_view file = __builtin_FILE(),
			 const size_t line = __builtin_LINE(),
#else
			 const std::string_view function = "",
			 const std::string_view file = "",
			 const size_t line = 0,
#endif
			 const uint32_t color = 0
)
			: name(name), function(function), file(file), line(line), color(color) {}
	};

	class IProfiler {
	public:
		virtual ~IProfiler() = default;

		// Frame boundary 
		// Call once per application frame (or per plugin tick).
		virtual void MarkFrame(std::string_view name) = 0;

		// CPU zones 
		// BeginZone / EndZone must be balanced. Prefer the RAII
		// ScopedZone helper below — don't call these directly.
		virtual ZoneHandle BeginZone(const ZoneDesc& desc) = 0;
		virtual void EndZone(ZoneHandle handle) = 0;

		// Memory tracking 
		//virtual void TrackAlloc(void* ptr, size_t size, std::string_view pool = {}) = 0;
		//virtual void TrackFree(void* ptr, std::string_view pool = {}) = 0;

		// Thread metadata
		virtual void SetThread(std::string_view name) = 0;

		// Capability query
		virtual std::string_view GetName() const = 0; // "Tracy", "Optick", …
		virtual bool IsActive() const = 0;
	};

	class ScopedZone {
	public:
		ScopedZone() = default;

		ScopedZone(const std::shared_ptr<IProfiler>& profiler, const ZoneDesc& desc) : _profiler(profiler.get()) {
			if (_profiler) _handle = _profiler->BeginZone(desc);
		}

		~ScopedZone() {
			if (_profiler && _handle) _profiler->EndZone(_handle);
		}

		ScopedZone(const ScopedZone&) = delete;
		ScopedZone& operator=(const ScopedZone&) = delete;

		ScopedZone(ScopedZone&& other) noexcept
			: _profiler(other._profiler)
			, _handle(other._handle) {
			other._profiler = nullptr;
			other._handle = {};
		}

		ScopedZone& operator=(ScopedZone&& other) noexcept {
			if (this != &other) {
				if (_profiler && _handle) _profiler->EndZone(_handle);

				_profiler = other._profiler;
				_handle = other._handle;

				other._profiler = nullptr;
				other._handle = {};
			}
			return *this;
		}

	private:
		IProfiler* _profiler = nullptr;
		ZoneHandle _handle = {};
	};
}