#pragma once

#include <plugify/path.hpp>

/**
 * @brief CrashLogs
 * simple C++ crashlog output library based on C++23's <stacktrace> header
 * call plugify::crashlogs::begin_monitoring() at the start of your program
 * and it will save a stack trace to a crash log if the program crashes!
 * a decent amount of this was copied & modified from backward.cpp (https://github.com/bombela/backward-cpp)
 * but with <stacktrace> it prints nice stack traces without needing to include pdbs and is also significantly simpler
 */
namespace plugify::crashlogs {
	/**
     * @brief Callback for when a crash log is written
     */
	using OnOutputCrashlog = void (*)(std::filesystem::path_view path, std::string_view trace);

	/**
     * @brief Begins crash monitoring. Crash logs will not be generated until begin_monitoring has been called
     */
	void BeginMonitoring();

	/**
     * @brief Set the folder path that crashlogs will be saved in.
     * If the folder doesn't exist, it will be created when the program crashes
     * @param folder_path The path to the folder where crash logs will be saved.
     */
	void SetCrashlogFolder(std::filesystem::path_view folder_path);

	/**
     * @brief Sets the format to use for crash log filenames. Default is "crash_{timestamp}.txt". Crashes are saved in the crashlog folder.
     * @param filename_format The format string for crash log filenames.
     */
	void SetCrashlogFilename(std::string_view filename_format);

	/**
     * @brief After a crash log is written, this optional callback is called with the full file path of where the crash log was written.
     * @param callback The callback function to be called after a crash log is written.
     */
	void SetOnWriteCrashlogCallback(OnOutputCrashlog callback);

	/**
     * @brief The most recently set crashlog header will be printed at the top of the crash log file.
     * @param message The header message to be printed at the top of the crash log file.
     */
	void SetCrashlogHeaderMessage(std::string_view message);

	/**
     * @brief Returns the current crashlog header message.
     * @return The current crashlog header message.
     */
	std::string_view GetCrashlogHeaderMessage();
}// namespace plugify::crashlogs
