#ifndef LOG_H
#define LOG_H

typedef enum {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_MESSAGE,
    LOG_LEVEL_DEBUG
} log_level_t;

#define debug(...) (log_message(LOG_LEVEL_DEBUG, __VA_ARGS__))
#define message(...) (log_message(LOG_LEVEL_MESSAGE, __VA_ARGS__))
#define warning(...) (log_message(LOG_LEVEL_WARNING, __VA_ARGS__))
#define error(...) (log_message(LOG_LEVEL_ERROR, __VA_ARGS__))
#define fatal(...) (log_message(LOG_LEVEL_FATAL, __VA_ARGS__))

extern void log_message(log_level_t level, const char *message, ...);

#endif

