#include "common.h"

uint64_t currentTimeMillis() {
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t when = (uint64_t) (now.tv_sec * 1000LL + now.tv_usec / 1000);
    return when;
}

char* duplicateString(const char* origStr) {
    size_t len = strlen(origStr);
    char *buffer = (char *) malloc(len + 1);
    buffer[len] = 0;
    memcpy(buffer, origStr, len);
    return buffer;
}