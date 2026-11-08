#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "plg/config.hpp"

namespace plugify {
	/**
	 * @typedef ZoneHandle
	 * @brief Identifies one open profiling zone.
	 *
	 * Handed out by IProfiler::BeginZone and given back to IProfiler::EndZone. A
	 * zero handle stands for no zone.
	 */
	using ZoneHandle = uint64_t;

	/**
	 * @class IProfiler
	 * @brief Interface for collecting timing information about the framework.
	 *
	 * An implementation receives frame boundaries and nested CPU zones, which it
	 * can forward to whatever profiler the host application already uses. The
	 * default instance does nothing, so the calls cost close to nothing when no
	 * profiler is registered.
	 *
	 * @see ScopedZone, PlugifyBuilder::WithProfiler
	 */
	class IProfiler {
	public:
		/**
		 * @brief Virtual destructor.
		 */
		virtual ~IProfiler() = default;

		// Frame boundary

		/**
		 * @brief Mark the boundary between two frames.
		 * @param name Name of the frame.
		 * @note Call once per application frame, or per plugin tick.
		 */
		virtual void MarkFrame(std::string_view name) = 0;

		// CPU zones

		/**
		 * @brief Open a CPU zone.
		 * @param name Name of the zone.
		 * @param location Source location the zone was opened at. Defaults to the current location.
		 * @return A handle identifying the open zone.
		 * @note BeginZone and EndZone have to be balanced. Prefer the RAII @ref ScopedZone helper
		 * over calling this directly.
		 */
		virtual ZoneHandle BeginZone(std::string_view name, const Location& location = Location::current()) = 0;

		/**
		 * @brief Close a CPU zone.
		 * @param handle The handle BeginZone returned.
		 */
		virtual void EndZone(ZoneHandle handle) = 0;
	};

	/**
	 * @class ScopedZone
	 * @brief RAII guard that keeps a profiling zone open for the current scope.
	 *
	 * Opens a zone on construction and closes it on destruction, which keeps
	 * BeginZone and EndZone balanced even when the scope is left early. A default
	 * constructed guard, or one built on a null profiler, does nothing.
	 *
	 * The type is move-only; a moved-from guard no longer closes the zone.
	 *
	 * @see IProfiler
	 */
	class ScopedZone {
	public:
		/**
		 * @brief Construct a guard that does nothing.
		 */
		ScopedZone() = default;

		/**
		 * @brief Open a zone for the current scope.
		 * @param profiler The profiler to report to; may be null, in which case nothing happens.
		 * @param name Name of the zone.
		 * @param location Source location the zone was opened at. Defaults to the current location.
		 */
		ScopedZone(const std::shared_ptr<IProfiler>& profiler, std::string_view name, const Location& location = Location::current()) : _profiler(profiler.get()) {
			if (_profiler) _handle = _profiler->BeginZone(name, location);
		}

		/**
		 * @brief Open a zone whose name is formatted on the spot.
		 * @tparam Args Types of the format arguments.
		 * @param profiler The profiler to report to; may be null, in which case nothing happens.
		 * @param location Source location the zone was opened at.
		 * @param fmt Format string for the name of the zone.
		 * @param args Format arguments.
		 */
		template <typename... Args>
		ScopedZone(const std::shared_ptr<IProfiler>& profiler, const Location& location, std::format_string<Args...> fmt, Args&&... args) : _profiler(profiler.get()) {
			if (_profiler) _handle = _profiler->BeginZone(std::format(fmt, std::forward<Args>(args)...), location);
		}

		/**
		 * @brief Close the zone, if one is open.
		 */
		~ScopedZone() {
			if (_profiler && _handle) _profiler->EndZone(_handle);
		}

		/**
		 * @brief Deleted copy constructor; a zone is closed exactly once.
		 */
		ScopedZone(const ScopedZone&) = delete;

		/**
		 * @brief Deleted copy assignment operator; a zone is closed exactly once.
		 */
		ScopedZone& operator=(const ScopedZone&) = delete;

		/**
		 * @brief Move constructor; takes the open zone over from @p other.
		 * @param other Another instance of ScopedZone.
		 */
		ScopedZone(ScopedZone&& other) noexcept
			: _profiler(other._profiler)
			, _handle(other._handle) {
			other._profiler = nullptr;
			other._handle = {};
		}

		/**
		 * @brief Move assignment operator; closes the current zone and takes over the one of
		 * @p other.
		 * @param other Another instance of ScopedZone.
		 * @return A reference to this instance after moving.
		 */
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
		IProfiler* _profiler = nullptr;  //!< The profiler the zone was opened on, null if there is none.
		ZoneHandle _handle = {};  //!< Handle of the open zone, zero if none is open.
	};
}
