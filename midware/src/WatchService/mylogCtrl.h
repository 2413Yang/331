
#ifndef _CLUSTERLOG_H_
#define _CLUSTERLOG_H_


/******************************************************************************
                    macro, define, typedef, enum
*******************************************************************************/
// We define these as macros so that the preprocessor
// has a chance to completely eliminate logging below
// the level we have chosen.
#define LOG_LEVEL_OFF       0
#define LOG_LEVEL_NOT       1
#define LOG_LEVEL_ERROR     2
#define LOG_LEVEL_WARN      3
#define LOG_LEVEL_INFO      4
#define LOG_LEVEL_DEBUG     5


// If not defined, it is off.
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO 
#endif

#ifndef LOG_PRINTF_STDOUT
#define LOG_PRINTF_STDOUT  (0)
#endif





//////////////////////////////////////
#if LOG_LEVEL > LOG_LEVEL_OFF
#include "mlog.h"
#endif

// Macros for all of the different available levels. This enables logging
// code for all unused levels to be completely eliminated by the preprocessor.

#define __LOG_SHORT_DO_ACTUAL_LOGGING(format, ...)                               \
do {                                                                                    \
        printf(format, ##__VA_ARGS__);                                                  \
} while (0)


#define __LOG_NO_ACTUAL_LOGGING do {} while(0)

#if LOG_LEVEL >= LOG_LEVEL_NOT
#if LOG_PRINTF_STDOUT
#define LOG_SFATAL(format, ...) __LOG_SHORT_DO_ACTUAL_LOGGING(format, ##__VA_ARGS__)
#else
#define LOG_SFATAL(format, ...)  LOGNOT(format, ##__VA_ARGS__)
#endif

#else
#define LOG_SFATAL(format, ...) __LOG_NO_ACTUAL_LOGGING
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#if LOG_PRINTF_STDOUT
#define LOG_SERROR(format, ...) __LOG_SHORT_DO_ACTUAL_LOGGING(format, ##__VA_ARGS__)
#else
#define LOG_SERROR(format, ...)  LOGERR(format, ##__VA_ARGS__)
#endif

#else
#define LOG_SERROR(format, ...) __LOG_NO_ACTUAL_LOGGING
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#if LOG_PRINTF_STDOUT
#define LOG_SWARN(format, ...) __LOG_SHORT_DO_ACTUAL_LOGGING(format, ##__VA_ARGS__)
#else
#define LOG_SWARN(format, ...)  LOGWAR(format, ##__VA_ARGS__)
#endif

#else
#define LOG_SWARN(format, ...) __LOG_NO_ACTUAL_LOGGING
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#if LOG_PRINTF_STDOUT
#define LOG_SINFO(format, ...) __LOG_SHORT_DO_ACTUAL_LOGGING(format, ##__VA_ARGS__)
#else
#define LOG_SINFO(format, ...)  LOGINF(format, ##__VA_ARGS__)
#endif

#else
#define LOG_SINFO(format, ...) __LOG_NO_ACTUAL_LOGGING
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#if LOG_PRINTF_STDOUT
#define LOG_SDEBUG(format, ...) __LOG_SHORT_DO_ACTUAL_LOGGING(format, ##__VA_ARGS__)
#else
#define LOG_SDEBUG(format, ...)  LOGDBG(format, ##__VA_ARGS__)
#endif

#else
#define LOG_SDEBUG(format, ...) __LOG_NO_ACTUAL_LOGGING
#endif


#endif /* _CLUSTERLOG_H_ */

