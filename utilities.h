#include <iostream>

#ifdef __linux__
    #include <unistd.h>
#else
    #include <windows.h>
#endif

void MSDelay(int ms) {
    #ifdef __linux__
        usleep(ms * 1000);
    #else
        Sleep(ms);
    #endif
}