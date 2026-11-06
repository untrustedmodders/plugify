#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "plg/config.hpp"

namespace plugify {
	using ZoneHandle = uint64_t;

	class IProfiler {
	public:
		virtual ~IProfiler() = default;

		// Frame boundary
		// Call once per application frame (or per plugin tick).
		virtual void MarkFrame(std::string_view name) = 0;

		// CPU zones
		// BeginZone / EndZone must be balanced. Prefer the RAII
		// ScopedZone helper below — don't call these directly.
		virtual ZoneHandle BeginZone(std::string_view name, const Location& location = Location::current()) = 0;
		virtual void EndZone(ZoneHandle handle) = 0;
	};

	class ScopedZone {
	public:
		ScopedZone() = default;

		ScopedZone(const std::shared_ptr<IProfiler>& profiler, std::string_view name, const Location& location = Location::current()) : _profiler(profiler.get()) {
			if (_profiler) _handle = _profiler->BeginZone(name, location);
		}

		template <typename... Args>
		ScopedZone(const std::shared_ptr<IProfiler>& profiler, const Location& location, std::format_string<Args...> fmt, Args&&... args) : _profiler(profiler.get()) {
			if (_profiler) _handle = _profiler->BeginZone(std::format(fmt, std::forward<Args>(args)...), location);
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
