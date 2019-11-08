#include <switch_min.h>
#include <string>
#include <queue>
#include <stdarg.h>
#include "logging.h"

LogPacket logger;

#define TIMEOUT_SECONDS 1
#define SLEEP 100000
#define TIMEOUT TIMEOUT_SECONDS * SLEEP

void log_message(const char *format, ...)
{
    int i = 0;
    while (logger.dirty && i < TIMEOUT)
    {
        svcSleepThread(SLEEP);
        i++;
    }
    logger.dirty = true;
    va_list args;
    va_start(args, format);
    vsnprintf(logger.buffer, 256, format, args);
    va_end(args);
}