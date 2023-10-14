#ifndef CRASH_SCREEN_H
#define CRASH_SCREEN_H

#include <stdio.h>

extern char *gPanicMessage;

#define panic(message) \
    do { \
        gPanicMessage = message; \
        *(volatile char *)0 = 0; \
    } while (0)

#endif // CRASH_SCREEN_H
