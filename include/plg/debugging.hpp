#pragma once

#include "plg/config.hpp"

#if __has_include(<debugging>)
#include <debugging>
#if defined(__cpp_lib_debugging) && __cpp_lib_debugging >= 202403L
#define PLUGIFY_HAS_STD_DEBUGGING 1
#else
#define PLUGIFY_HAS_STD_DEBUGGING 0
#endif
#else
#define PLUGIFY_HAS_STD_DEBUGGING 0
#endif

#if !PLUGIFY_HAS_STD_DEBUGGING
#if PLUGIFY_PLATFORM_WINDOWS
#include <windows.h>
#include <intrin.h>
#elif PLUGIFY_PLATFORM_SWITCH
#include <nn/diag/diag_Debugger.h>
#elif PLUGIFY_PLATFORM_ORBIS || PLUGIFY_PLATFORM_PROSPERO
#include <libdbg.h>
#elif PLUGIFY_PLATFORM_APPLE
#include <mach/mach_init.h>
#include <mach/task.h>
#elif PLUGIFY_PLATFORM_LINUX
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace plg {

#if PLUGIFY_PLATFORM_WINDOWS

	inline bool is_debugger_present() noexcept {
		return (IsDebuggerPresent() != FALSE);
	}

#elif PLUGIFY_PLATFORM_SWITCH

	inline bool is_debugger_present() noexcept {
		return nn::diag::IsDebuggerAttached();
	}

#elif PLUGIFY_PLATFORM_ORBIS || PLUGIFY_PLATFORM_PROSPERO

	inline bool is_debugger_present() noexcept {
		return sceDbgIsDebuggerAttached() != 0;
	}

#elif PLUGIFY_PLATFORM_APPLE

	inline bool is_debugger_present() noexcept {
		mach_msg_type_number_t count = 0;
		exception_mask_t masks[EXC_TYPES_COUNT];
		mach_port_t ports[EXC_TYPES_COUNT];
		exception_behavior_t behaviors[EXC_TYPES_COUNT];
		thread_state_flavor_t flavors[EXC_TYPES_COUNT];
		exception_mask_t mask = EXC_MASK_ALL & ~(EXC_MASK_RESOURCE | EXC_MASK_GUARD);

		kern_return_t result = task_get_exception_ports(mach_task_self(), mask, masks, &count, ports, behaviors, flavors);
		if (result != KERN_SUCCESS)
			return false;

		for (mach_msg_type_number_t i = 0; i < count; ++i)
			if (MACH_PORT_VALID(ports[i]))
				return true;

		return false;
	}

#elif PLUGIFY_PLATFORM_LINUX

	namespace detail {
		namespace debugging {
			inline bool parse_proc_status_line(char* buf, size_t size) noexcept {
				if (std::strncmp(buf, "TracerPid:\t", 11) != 0) {
					return false;
				}
				unsigned long long pid = 0;
				size_t pos = 11;
				while (pos < size) {
					auto byte = static_cast<unsigned char>(buf[pos]);
					if (!(static_cast<unsigned char>('0') <= byte && byte <= static_cast<unsigned char>('9'))) {
						return false;
					}
					uint8_t digit = static_cast<uint8_t>(byte - static_cast<unsigned char>('0'));
					pid = (pid * 10) + digit;
					pos++;
				}
				return (pid != 0);
			}

			inline bool parse_proc_status() noexcept {
				int olderrno = errno;
				bool detected_debugger = false;
				int error = 0;
				int fd = -1;
				bool open_done = false;
				while (!open_done) {
					int res = open("/proc/self/status", O_RDONLY);
					if (res >= 0) {
						fd = res;
						open_done = true;
					} else if (errno != EINTR) {
						error = errno;
						open_done = true;
					}
				}
				if (error != 0) {
					errno = olderrno;
					return false;
				}
				if (fd < 0) {
					errno = olderrno;
					return false;
				}
				bool eof = false;
				uint8_t iobuf[1024];
				size_t iobuf_size = 0;
				char linebuf[128];
				size_t linebuf_size = 0;
				while (!eof) {
					ssize_t bytes_read = read(fd, iobuf, sizeof(iobuf));
					if (bytes_read == -1) {
						if (errno == EINTR) {
							continue;
						}
					} else if (bytes_read == 0) {
						eof = true;
					}
					iobuf_size = static_cast<size_t>(bytes_read);
					for (size_t i = 0; i < iobuf_size; ++i) {
						if (iobuf[i] == static_cast<unsigned char>('\n')) {
							if (parse_proc_status_line(linebuf, linebuf_size)) {
								detected_debugger = true;
							}
							linebuf_size = 0;
						} else {
							if (linebuf_size < sizeof(linebuf)) {
								linebuf[linebuf_size] = static_cast<char>(iobuf[i]);
								linebuf_size++;
							}
						}
					}
					if (linebuf_size > 0) {
						if (parse_proc_status_line(linebuf, linebuf_size)) {
							detected_debugger = true;
						}
						linebuf_size = 0;
					}
				}
				bool close_done = false;
				while (!close_done) {
					int res = close(fd);
					if (res == 0) {
						fd = -1;
						close_done = true;
					} else if (errno != EINTR) {
						error = errno;
						close_done = true;
					}
				}
				if (error != 0) {
					errno = olderrno;
					return false;
				}
				errno = olderrno;
				return detected_debugger;
			}

		} // namespace debugging
	} // namespace detail

	inline bool is_debugger_present() noexcept {
		return plg::detail::debugging::parse_proc_status();
	}

#else

	inline bool is_debugger_present() noexcept {
		return false;
	}

#endif

	inline void breakpoint() noexcept {
#if PLUGIFY_COMPILER_MSVC
		__debugbreak();
#elif PLUGIFY_COMPILER_CLANG
		__builtin_debugtrap();
#elif PLUGIFY_COMPILER_GCC
		__builtin_trap();
#else
#	error "Unsupported platform for breakpoint"
#endif
	}

	inline void breakpoint_if_debugging() noexcept {
		if (plg::is_debugger_present()) {
			plg::breakpoint();
		}
	}
} // namespace plg

namespace std {
	using plg::breakpoint;
	using plg::breakpoint_if_debugging;
	using plg::is_debugger_present;
} // namespace std

#endif // !PLUGIFY_HAS_STD_DEBUGGING
