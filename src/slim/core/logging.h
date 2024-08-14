#pragma once

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>  // isspace

#include "./base.h"

#define SLIM_DO_LOG_WARNINGS 1
#define SLIM_DO_LOG_INFO 1

#ifdef NDEBUG
    #define SLIM_DO_LOG_DEBUG 0
    #define SLIM_DO_LOG_TRACE 0
#else
    #define SLIM_DO_LOG_DEBUG 1
    #define SLIM_DO_LOG_TRACE 1
#endif

enum SlimLogLevel {
    SLIM_LOG_LEVEL_FATAL = 0, // Stop the application when hit
    SLIM_LOG_LEVEL_ERROR = 1, // Critical runtime issue causing the application to run incorrectly
    SLIM_LOG_LEVEL_WARNING = 2, // Non-critical runtime issue causing the application to run less than optimally
    SLIM_LOG_LEVEL_INFO = 3, // Informational details (not an error)
    SLIM_LOG_LEVEL_DEBUG = 4, // For debugging
    SLIM_LOG_LEVEL_TRACE = 5 // for detailed debugging (verbose)
};


const char* SLIM_LOG_LEVEL_NAMES[6] = {
    "\n[FATAL]: ",
    "\n[ERROR]: ",
    "\n[WARN ]: ",
    "\n[INFO ]: ",
    "\n[DEBUG]: ",
    "\n[TRACE]: "
};

#define SLIM_LOG_MESSAGE_BUFFER_LENGTH (16*1024)
#define SLIM_LOG_MESSAGE_PREFIX_LENGTH 9
#define SLIM_LOG_MESSAGE_FORMAT_LENGTH (SLIM_LOG_MESSAGE_BUFFER_LENGTH - SLIM_LOG_MESSAGE_PREFIX_LENGTH)

char SLIM_LOG_MESSAGE_BUFFER[SLIM_LOG_MESSAGE_BUFFER_LENGTH];
char* SLIM_LOG_FORMAT_BUFFER = SLIM_LOG_MESSAGE_BUFFER + SLIM_LOG_MESSAGE_PREFIX_LENGTH;

//SLIM_API
void SLIM_LOG(SlimLogLevel log_level, const char* msg, ...) {
    bool is_error = log_level < SLIM_LOG_LEVEL_WARNING;
    for (int i = 0; i < SLIM_LOG_MESSAGE_PREFIX_LENGTH; i++) SLIM_LOG_MESSAGE_BUFFER[i] = SLIM_LOG_LEVEL_NAMES[log_level][i];

    va_list arg_ptr;
//    __builtin_va_list arg_ptr;
    va_start(arg_ptr, msg);
    int len = vsnprintf(SLIM_LOG_FORMAT_BUFFER, SLIM_LOG_MESSAGE_FORMAT_LENGTH, msg, arg_ptr);
    SLIM_LOG_FORMAT_BUFFER[len] = 0;
    va_end(arg_ptr);

    // Prepend log level to message.
//    string_format(out_message, "%s%s\n", level_strings[log_level], out_message);

//    // Pass along to console consumers.
//    console_write_line(log_level, out_message);

    // Print accordingly
    if (is_error) {
        os::printError(SLIM_LOG_MESSAGE_BUFFER, log_level);
    } else {
        os::print(SLIM_LOG_MESSAGE_BUFFER, log_level);
    }

//    append_to_log_file(out_message);
}

void SLIM_LOG_ASSERT_FAILURE(const char* expr, const char* msg, const char* file_path, i32 line_number) {
    SLIM_LOG(SLIM_LOG_LEVEL_FATAL, "Failed assertion: %s, message: '%s', file: %s, line: %d\n", expr, msg, file_path, line_number);
}

//bool SLIM_INIT_LOGGING(u64* size, void* state, void* conf);
//void SLIM_SHUTDOWN_LOGGING(void* state);



//
//struct logger_system_state {
//    file_handle log_file_handle;
//};
//
//logger_system_state* state_ptr;
//
//void append_to_log_file(const char* message) {
//    if (state_ptr && state_ptr->log_file_handle.is_valid) {
//        // Since the message already contains a '\n', just write the bytes directly.
//        u64 length = string_length(message);
//        u64 written = 0;
//        if (!filesystem_write(&state_ptr->log_file_handle, length, message, &written)) {
//            platform_console_write_error("ERROR writing to console.log.", SLIM_LOG_LEVEL_ERROR);
//        }
//    }
//}

//bool logging_initialize(u64* size, void* state, void* conf) {
//    *size = sizeof(logger_system_state);
//    if (state == 0) {
//        return true;
//    }
//
//    state_ptr = state;
//
//    // Create new/wipe existing log file, then open it.
//    if (!filesystem_open("console.log", FILE_MODE_WRITE, false, &state_ptr->log_file_handle)) {
//        platform_console_write_error("ERROR: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
//        return false;
//    }
//
//    return true;
//}
//
//void logging_shutdown(void* state) {
//    state_ptr = 0;
//}


#define SLIM_LOG_FATAL(msg, ...) SLIM_LOG(SLIM_LOG_LEVEL_FATAL, msg, ##__VA_ARGS__);
#ifndef SLIM_LOG_ERROR
#define SLIM_LOG_ERROR(msg, ...) SLIM_LOG(SLIM_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__);
#endif

#if SLIM_DO_LOG_WARNINGS == 1
#define SLIM_LOG_WARNING(msg, ...) SLIM_LOG(SLIM_LOG_LEVEL_WARNING, msg, ##__VA_ARGS__);
#else
#define SLIM_LOG_WARNING(msg, ...)
#endif

#if SLIM_DO_LOG_INFO == 1
#define SLIM_LOG_INFO(msg, ...) SLIM_LOG(SLIM_LOG_LEVEL_INFO, msg, ##__VA_ARGS__);
#else
#define SLIM_LOG_INFO(msg, ...)
#endif

#if SLIM_DO_LOG_DEBUG == 1
#define SLIM_LOG_DEBUG(msg, ...) SLIM_LOG(SLIM_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__);
#else
#define SLIM_LOG_DEBUG(msg, ...)
#endif

#if SLIM_DO_LOG_TRACE == 1
#define SLIM_LOG_TRACE(msg, ...) SLIM_LOG(SLIM_LOG_LEVEL_TRACE, msg, ##__VA_ARGS__);
#else
#define SLIM_LOG_TRACE(msg, ...)
#endif