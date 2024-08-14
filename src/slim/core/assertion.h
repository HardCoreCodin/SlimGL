#pragma once

#include "./base.h"

#define SLIM_ASSERTIONS_ENABLED

#ifdef SLIM_ASSERTIONS_ENABLED
    #if defined(COMPILER_MSVC)
        #include <intrin.h>
        #define debugBreak() __debugbreak()
    #else
        #define debugBreak() __builtin_trap()
    #endif

    #ifdef SLIM_DO_LOG_INFO
        //    SLIM_API
        void SLIM_LOG_ASSERT_FAILURE(const char* expr, const char* msg, const char* file_path, i32 line_number);
    #else
        #define SLIM_LOG_ASSERT_FAILURE(expr, msg, file_path, line_number)
    #endif

    #define SLIM_ASSERT(expr) {if (expr) {} else { SLIM_LOG_ASSERT_FAILURE(#expr, "", __FILE__, __LINE__); debugBreak(); }}
    #define SLIM_ASSERT_MSG(expr, msg) { if (expr) {} else { SLIM_LOG_ASSERT_FAILURE(#expr, msg, __FILE__, __LINE__); debugBreak(); }}

    #ifndef NDEBUG
        #define SLIM_ASSERT_DEBUG(expr) { if (expr) {} else { SLIM_LOG_ASSERT_FAILURE(#expr, "", __FILE__, __LINE__); debugBreak(); }}
    #else
        #define SLIM_ASSERT_DEBUG(expr)
    #endif
#else
    #define SLIM_ASSERT(expr)
    #define SLIM_ASSERT_MSG(expr, msg)
    #define SLIM_ASSERT_DEBUG(expr)
#endif