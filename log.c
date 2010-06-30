#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static char *levels[] = {
    "FATAL",
    "ERROR",
    "WARNING",
    "MESSAGE",
    "DEBUG"
};

extern void log_message(log_level_t level, const char *message, ...)
{
    FILE *out = (level >= LOG_LEVEL_MESSAGE) ? stdout : stderr;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    fprintf(out, "%i:%i:%i - %s - ", tm->tm_hour, tm->tm_min, tm->tm_sec,
        levels[level]);
    
    va_list ap;
    va_start(ap, message);

    vfprintf(out, message, ap);
}

