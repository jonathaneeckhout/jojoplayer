#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

// Function prototypes for logging system initialization and cleanup
void logging_init(const char *name, int log_level);
void logging_cleanup();
void logging_set_log_level(int log_level);

// Helper macro to get only the file name from the __FILE__ macro
#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// Logging macros with only the file name and timestamp
#define log_error(format, ...) \
    syslog(LOG_ERR, format " [%s:%d]", ##__VA_ARGS__, FILENAME, __LINE__)
#define log_warning(format, ...) \
    syslog(LOG_WARNING, format " [%s:%d]", ##__VA_ARGS__, FILENAME, __LINE__)
#define log_info(format, ...) \
    syslog(LOG_INFO, format " [%s:%d]", ##__VA_ARGS__, FILENAME, __LINE__)
#define log_notice(format, ...) \
    syslog(LOG_NOTICE, format " [%s:%d]", ##__VA_ARGS__, FILENAME, __LINE__)
#define log_debug(format, ...) \
    syslog(LOG_DEBUG, format " [%s:%d]", ##__VA_ARGS__, FILENAME, __LINE__)

#define when_null_log_error(x, l, ...) \
    if ((x) == NULL)                   \
    {                                  \
        log_error(__VA_ARGS__);        \
        goto l;                        \
    }
#define when_not_null_log_error(x, l, ...) \
    if ((x) != NULL)                       \
    {                                      \
        log_error(__VA_ARGS__);            \
        goto l;                            \
    }
#define when_true_log_error(x, l, ...) \
    if ((x))                           \
    {                                  \
        log_error(__VA_ARGS__);        \
        goto l;                        \
    }
#define when_false_log_error(x, l, ...) \
    if (!(x))                           \
    {                                   \
        log_error(__VA_ARGS__);         \
        goto l;                         \
    }
#define when_failed_log_error(x, l, ...) \
    if ((x) != 0)                        \
    {                                    \
        log_error(__VA_ARGS__);          \
        goto l;                          \
    }
#define when_str_empty_log_error(x, l, ...) \
    if ((x) == NULL || *(x) == 0)           \
    {                                       \
        log_error(__VA_ARGS__);             \
        goto l;                             \
    }

#define when_null_log_warning(x, l, ...) \
    if ((x) == NULL)                     \
    {                                    \
        log_warning(__VA_ARGS__);        \
        goto l;                          \
    }
#define when_not_null_log_warning(x, l, ...) \
    if ((x) != NULL)                         \
    {                                        \
        log_warning(__VA_ARGS__);            \
        goto l;                              \
    }
#define when_true_log_warning(x, l, ...) \
    if ((x))                             \
    {                                    \
        log_warning(__VA_ARGS__);        \
        goto l;                          \
    }
#define when_false_log_warning(x, l, ...) \
    if (!(x))                             \
    {                                     \
        log_warning(__VA_ARGS__);         \
        goto l;                           \
    }
#define when_failed_log_warning(x, l, ...) \
    if ((x) != 0)                          \
    {                                      \
        log_warning(__VA_ARGS__);          \
        goto l;                            \
    }
#define when_str_empty_log_warning(x, l, ...) \
    if ((x) == NULL || *(x) == 0)             \
    {                                         \
        log_warning(__VA_ARGS__);             \
        goto l;                               \
    }

#endif // LOGGING_H
