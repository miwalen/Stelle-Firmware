#include "Logger.h"

#include <cstdarg>
#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace stelle {

extern SemaphoreHandle_t mutexLog;

namespace {

constexpr size_t LOG_LINE_MAX = 192;

// Satu tempat yang menyentuh mutexLog + logger tujuan.
void writeLine(Logger& target, const char* prefix, const char* fmt, va_list args) {
    char line[LOG_LINE_MAX];
    vsnprintf(line, sizeof(line), fmt, args);

    char prefixed[LOG_LINE_MAX + 16];
    snprintf(prefixed, sizeof(prefixed), "[%s] %s", prefix, line);

    if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {
        target.log(prefixed);
        xSemaphoreGive(mutexLog);
    }
}

}  // namespace

void logI(Logger& target, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    writeLine(target, "INFO", fmt, args);
    va_end(args);
}

void logW(Logger& target, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    writeLine(target, "WARN", fmt, args);
    va_end(args);
}

void logE(Logger& target, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    writeLine(target, "ERROR", fmt, args);
    va_end(args);
}

} // namespace stelle
