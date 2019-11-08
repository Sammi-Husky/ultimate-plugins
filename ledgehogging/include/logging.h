#include <string>
#include <queue>
#include <stdarg.h>

#ifndef LOGGING_H

struct LogPacket
{
    bool dirty = false;
    bool to_sd = false;
    char buffer[256];
};

extern LogPacket logger;

void log_message(const char* message, ...);
#endif