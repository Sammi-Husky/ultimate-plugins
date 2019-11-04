#include <switch_min.h>
#include <string>
#include <queue>
#include <stdarg.h>
#include "logging.h"

LogPacket logger;

// Timeout in miliseconds
#define TIMEOUT 1000

void log_message(const char* format, ...) {
    int i=0;
    while(logger.dirty && i < TIMEOUT) {
        svcSleepThread(100000000);
        i++;
    }
    logger.dirty = true;
    va_list args;
    va_start(args, format);
    vsnprintf(logger.buffer, 256, format, args);
    va_end(args);
}