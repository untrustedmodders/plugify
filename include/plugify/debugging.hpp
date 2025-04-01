#pragma once

#include "macro.hpp"

#if COMPILER_SUPPORTS_STACKTRACE
#include <debugging>
#endif // C++26

#if !COMPILER_SUPPORTS_STACKTRACE
#if PLUGIFY_PLATFORM_LINUX
#include <cerrno>
#include <cstring>
#endif
#endif // !C++26

#if !COMPILER_SUPPORTS_STACKTRACE
#if PLUGIFY_PLATFORM_LINUX
#include <fcntl.h>
#include <unistd.h>
#endif
#endif // !C++26

#if !COMPILER_SUPPORTS_STACKTRACE
#if PLUGIFY_PLATFORM_WINDOWS
#include <windows.h>
#endif
#endif // !C++26

#if !COMPILER_SUPPORTS_STACKTRACE
#if PLUGIFY_COMPILER_MSVC
#include <intrin.h>
#endif
#endif // !C++26

#if !COMPILER_SUPPORTS_STACKTRACE
#if PLUGIFY_PLATFORM_POSIX
#include <csignal>
#endif
#endif // !C++26


namespace plg {
#if COMPILER_SUPPORTS_STACKTRACE
	using std::breakpoint;
	using std::breakpoint_if_debugging;
	using std::is_debugger_present;

#else // !C++26

#if PLUGIFY_PLATFORM_WINDOWS

	inline bool is_debugger_present() noexcept {
		return (IsDebuggerPresent() != FALSE);
	}

	PLUGIFY_FORCE_INLINE void breakpoint() noexcept {
#if PLUGIFY_COMPILER_MSVC
		__debugbreak();
#elif PLUGIFY_COMPILER_CLANG
		__builtin_debugtrap();
#elif PLUGIFY_COMPILER_GCC
		__builtin_trap();
#else
		DebugBreak();
#endif
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
						if (static_cast<unsigned char>(iobuf[i]) == static_cast<unsigned char>('\n')) {
							if (parse_proc_status_line(linebuf, linebuf_size)) {
								detected_debugger = true;
							}
							linebuf_size = 0;
						} else {
							if (linebuf_size < sizeof(linebuf[)) {
								linebuf[linebuf_size] = static_cast<char>(static_cast<unsigned char>(iobuf[i]));
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

	PLUGIFY_NOINLINE inline bool is_debugger_present() noexcept {
		return plg::detail::debugging::parse_proc_status();
	}

	PLUGIFY_FORCE_INLINE void breakpoint() noexcept {
#if PLUGIFY_COMPILER_CLANG
		__builtin_debugtrap();
#elif PLUGIFY_COMPILER_GCC
		__builtin_trap();
#else
		raise(SIGTRAP);
#endif
	}

#else

	inline bool is_debugger_present() noexcept {
		return false;
	}

	PLUGIFY_FORCE_INLINE void breakpoint() noexcept {
#if PLUGIFY_COMPILER_MSVC
		__debugbreak();
#elif PLUGIFY_COMPILER_CLANG
		__builtin_debugtrap();
#elif PLUGIFY_COMPILER_GCC
		__builtin_trap();
#elif PLUGIFY_PLATFORM_POSIX
        raise(SIGTRAP);
#endif
	}

#endif

	PLUGIFY_FORCE_INLINE void breakpoint_if_debugging() noexcept {
		if (plg::is_debugger_present()) {
			plg::breakpoint();
		}
	}

#endif // C++26
} // namespace plg