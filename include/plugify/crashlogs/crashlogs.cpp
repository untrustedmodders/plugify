#include "crashlogs.hpp"

#include <stacktrace>
//needed to get a stack trace
#if PLUGIFY_STACKTRACE_SUPPORT
#include <stacktrace>
#else
#include <cpptrace/cpptrace.hpp>
#endif // PLUGIFY_STACKTRACE_SUPPORT

//needed for threading (threading needed to be able to output a call stack during a stack overflow)
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

//needed for being able to output a timestampped crash log
#include <filesystem>
#include <fstream>
#include <chrono>

//needed for being able to get into the crash handler on a crash
#include <csignal>
#include <exception>
#include <cstdlib>

#if PLUGIFY_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// a decent amount of this was copied/modified from backward.cpp (https://github.com/bombela/backward-cpp)
// mostly the stuff related to actually getting crash handlers on crashes
// and the thread which is SOLELY there to be able to write a log on a stack overflow,
// since otherwise there is not enough stack space to output the stack trace
// main difference here is utilizing C++23 <stacktrace> header for generating stack traces
// and using <atomic> and a few other more recent C++ features if we're gonna be using C++23 anyway

namespace plugify::crashlogs {
	//information for where to save stack traces
#if PLUGIFY_STACKTRACE_SUPPORT
	static std::stacktrace trace;
#else
	static cpptrace::stacktrace trace;
#endif // PLUGIFY_STACKTRACE_SUPPORT
	static std::string header_message;
	static int crash_signal = 0; // 0 is not a valid signal id
	static std::filesystem::path output_folder;
	static std::string filename = "crash_{timestamp}.txt";
	static on_write_crashlog on_output_crashlog = nullptr;

	//thread stuff
	static std::mutex mutex;
	static std::condition_variable cv;
	static std::thread output_thread;
	enum class program_status {
		running = 0,
		crashed = 1,
		ending = 2,
		normal_exit = 3,
	};
	static std::atomic<program_status> status = program_status::running;

	//public interface (see header for documentation)
	void set_crashlog_folder(std::string_view folder_path) {
		output_folder = folder_path;
	}

	void set_crashlog_filename(std::string_view filename_format) {
		filename = filename_format;
	}

	void set_on_write_crashlog_callback(on_write_crashlog callback) {
		on_output_crashlog = callback;
	}

	void set_crashlog_header_message(std::string_view message) {
		std::unique_lock<std::mutex> lk(mutex);
		if (status != program_status::running) return;
		header_message = message;
	}

	std::string_view get_crashlog_header_message() {
		return header_message;
	}

	static std::filesystem::path get_log_filepath();
	static const char* try_get_signal_name(int signal);

	//output the crashlog file after a crash has occured
	static void output_crash_log() {
		std::filesystem::path path = get_log_filepath();
		std::ofstream log(path);
		if (!header_message.empty())
			log << header_message << std::endl;
		if (crash_signal != 0) {
			log << "Received signal " << crash_signal << " " << try_get_signal_name(crash_signal) << std::endl;
		}
		log << trace;
		log.close();

		if (on_output_crashlog) {
#if PLUGIFY_STACKTRACE_SUPPORT
			on_output_crashlog(path.string(), std::to_string(trace));
#else
			on_output_crashlog(path.string(), trace.to_string());
#endif // PLUGIFY_STACKTRACE_SUPPORT
		}
	}

	//get the current timestamp as a string, for the crash log filename
	static std::string current_timestamp() {
		auto now = std::chrono::system_clock::now();
		auto t = std::chrono::system_clock::to_time_t(now);
		std::tm time{};
#if PLUGIFY_PLATFORM_WINDOWS
		localtime_s(&time, &t); // Windows-specific
#else
		localtime_r(&t, &time); // POSIX-compliant
#endif
		std::string buffer(80, '\0');
		size_t res = std::strftime(buffer.data(), buffer.size(), "%Y-%m-%d-%H-%M-%S", &time);
		if (!res)
			return "strftime error";
		buffer.resize(res);
		return buffer;
	}

	//utility function needed for crash log timestamps (replace {timestamp} in filename format with the timestamp string)
	static std::string replace_substr(std::string str, const std::string& search, const std::string& replace) {
		if (search.empty())
			return str;

		size_t pos = 0;
		while((pos = str.find(search, pos)) != std::string::npos) {
			str.replace(pos, search.length(), replace);
			pos += replace.length();
		}
		return str;
	}

	//get the crash log filename
	std::filesystem::path get_log_filepath() {
		std::string timestampstr = current_timestamp();
		std::string timestampped_filename = replace_substr(filename, "{timestamp}", timestampstr);

		std::filesystem::path filepath = output_folder / timestampped_filename;

		//ensure the crash log folder exists. error code is here to supporess errors... since we're in an error handler already
		//at default settings this errors because an empty path is specified... lol. but we dont want to create a folder in that case anyway
		std::error_code err;
		std::filesystem::create_directories(output_folder, err);

		return filepath;
	}

	//using a thread here is a hack to get stack space in the case where the crash is a stack overflow
	//this hack was borrowed from backward.cpp
	static void crash_handler_thread() {
		//wait for the program to crash or exit normally
		std::unique_lock<std::mutex> lk(mutex);
		cv.wait(lk, [] { return status != program_status::running; });
		lk.unlock();

		//if it crashed, output the crash log
		if (status == program_status::crashed) {
			output_crash_log();
		}

		//alert the crashing thread we're done with the crash log so it can finish crashing
		status = program_status::ending;
		cv.notify_one();
	}

	static inline void crash_handler() {
		//if we crashed during a crash... ignore lol
		if (status != program_status::running) return;

		//save the stacktrace
#if PLUGIFY_STACKTRACE_SUPPORT
		trace = std::stacktrace::current();
#else
		trace = cpptrace::generate_trace();
#endif // PLUGIFY_STACKTRACE_SUPPORT

		//resume the monitoring thread
		status = program_status::crashed;
		cv.notify_one();

		//wait for the crash log to finish writing
		std::unique_lock<std::mutex> lk(mutex);
		cv.wait(lk, [] { return status != program_status::crashed; });
	}

	//Try to get the string representation of a signal identifier, return an empty string if none is found.
	//This only covers the signals from the C++ std lib and none of the POSIX or OS specific signal names!
	static const char* try_get_signal_name(int signal) {
		switch (signal) {
			case SIGTERM:
				return "SIGTERM";
			case SIGSEGV:
				return "SIGSEGV";
			case SIGINT:
				return "SIGINT";
			case SIGILL:
				return "SIGILL";
			case SIGABRT:
				return "SIGABRT";
			case SIGFPE:
				return "SIGFPE";
			default:
				return "";
		}
	}

	//various callbacks needed to get into the crash handler during a crash (borrowed from backward.cpp)
	static inline void signal_handler(int signal) {
		crash_signal = signal;
		crash_handler();
		std::quick_exit(1);
	}

	static inline void terminator() {
		crash_handler();
		std::quick_exit(1);
	}

#if PLUGIFY_PLATFORM_WINDOWS
	__declspec(noinline) static LONG WINAPI exception_handler(EXCEPTION_POINTERS*) {
		//TODO consider writing additional output from info to header_reason
		crash_handler();
		return EXCEPTION_CONTINUE_SEARCH;
	}
	static void __cdecl invalid_parameter_handler(const wchar_t*,const wchar_t*,const wchar_t*,unsigned int,uintptr_t) {
		//TODO consider writing additional output from info to header_reason
		crash_handler();
		abort();
	}
#endif

	//callback needed during a normal exit to shut down the thread
	static inline void normal_exit() {
		status = program_status::normal_exit;
		cv.notify_one();
		output_thread.join();
	}

#if PLUGIFY_PLATFORM_WINDOWS
	//set up all the callbacks needed to get into the crash handler during a crash (borrowed from backward.cpp)
	void begin_monitoring() {
		output_thread = std::thread(crash_handler_thread);

		SetUnhandledExceptionFilter(exception_handler);
		std::signal(SIGABRT, signal_handler);
		std::set_terminate(terminator);
		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
		_set_purecall_handler(terminator);
		_set_invalid_parameter_handler(&invalid_parameter_handler);

		std::atexit(normal_exit);
	}
#endif // PLUGIFY_PLATFORM_WINDOWS

#if PLUGIFY_PLATFORM_UNIX
	//set up all the callbacks needed to get into the crash handler during a crash (borrowed from backward.cpp)

	void begin_monitoring() {
		output_thread = std::thread(crash_handler_thread);

		std::signal(SIGABRT, signal_handler);
		std::signal(SIGSEGV, signal_handler);
		std::signal(SIGILL, signal_handler);

		std::set_terminate(terminator);

		std::atexit(normal_exit);
	}
#endif // PLUGIFY_PLATFORM_UNIX

}// namespace plugify::crashlogs