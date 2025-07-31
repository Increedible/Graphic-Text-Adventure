#include <random>
#include <ctime>
#ifdef _WIN32
    #include <windows.h>
#elif __linux__
    #include <unistd.h>
#else
    #error "Unsupported platform"
#endif

#include "utilities.h"

void MSDelay(int ms) {
    #ifdef __linux__
        usleep(ms * 1000);
    #else
        Sleep(ms);
    #endif
}

static std::mt19937 rng(time(0));

// [l, r)
int randomnum(int l, int r) {
    std::uniform_int_distribution gen(l, r-1);
    return gen(rng);
}
// [0,r)
int randomnum(int r) {
    return randomnum(0, r);
}

std::string toString(char c) {
    std::string s(1, c);
    return s;
}