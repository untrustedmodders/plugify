#pragma once

#include <memory>
#include <string_view>

#include "plg/config.hpp"

namespace plugify {
	using ZoneHandle = uint64_t;

	struct ZoneInfo {
		std::string_view name;
		std::string_view function;
		std::string_view file;
		size_t line;
		uint32_t color;

		ZoneInfo(
			std::string_view name_ = "",
			std::string_view function_ = PLUGIFY_FUNCTION,
			std::string_view file_ = PLUGIFY_FILE,
			size_t line_ = PLUGIFY_LINE,
			uint32_t color_ = 0
		) noexcept
			: name(name_)
			, function(function_)
			, file(file_)
			, line(line_)
			, color(color_) {
		}
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
		virtual ZoneHandle BeginZone(const ZoneInfo& desc = {}) = 0;
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

		ScopedZone(const std::shared_ptr<IProfiler>& profiler, const ZoneInfo& desc = {}) : _profiler(profiler.get()) {
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