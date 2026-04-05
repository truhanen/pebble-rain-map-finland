#include "logging.h"

#include "types.h"

void log_heap_bytes() {
    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "Heap bytes used: %d, free: %d",
        heap_bytes_used(),
        heap_bytes_free()
    );
}

void log_time(time_t timestamp) {
    tm timeinfo = *localtime(&timestamp);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %z", &timeinfo);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", buffer);
}
